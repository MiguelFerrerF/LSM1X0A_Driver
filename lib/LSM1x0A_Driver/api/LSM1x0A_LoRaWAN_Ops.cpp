#include "../LSM1x0A_Controller.h"
#include "esp_timer.h"


// =========================================================================
// NETWORK LORAWAN OPERATIONS (JOIN, SEND, LINKCHECK)
// =========================================================================

bool LSM1x0A_LoRaWAN::join(LsmJoinMode joinMode, uint32_t timeoutMs)
{
  if (!_controller)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN, joinMode);

  // Save the desired join mode to use it in recovery if necessary
  _joinMode = joinMode;

  // Clear previous flags
  _controller->clearEvents(LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL);

  LSM_LOG_INFO("LORA", "Initiating LoRaWAN Join (%s)", joinMode == LsmJoinMode::OTAA ? "OTAA" : "ABP");

  // Send the command and wait for the initial OK
  AtError err = _controller->sendCommand(cmd, 2000, 1);
  if (err != AtError::OK) {
    LSM_LOG_ERROR("LORA", "Failed to dispatch Join request.");
    return false;
  }
  
  LSM_LOG_INFO("LORA", "Join request dispatched successfully.");

  // Now wait asynchronously for the URC event
  uint32_t result = _controller->waitForEvent(LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL, timeoutMs);

  if (result & LSM_EVT_JOIN_SUCCESS) {
    if (_pendingChannelMask) {
      setChannelMask(_cachedBand, _cachedSubBandMask);
    }
    LSM_LOG_INFO("LORA", "Join SUCCESS.");
    return true; // Complete success
  }
  else if (result & LSM_EVT_JOIN_FAIL) {
    LSM_LOG_ERROR("LORA", "Join FAILED (e.g. bad credentials or no coverage).");
    return false; // Join failure (e.g., bad credentials, no coverage, etc.)
  }

  // If we reach here, it's an unexpected MAC layer timeout
  LSM_LOG_ERROR("LORA", "Join TIMEOUT. Module hung without emitting failure event. Forcing recovery.");
  _controller->recoverModule();
  return false; // Timeout
}

bool LSM1x0A_LoRaWAN::sendData(uint8_t port, const char* data, bool confirmed, uint32_t timeoutMs)
{
  if (!_controller)
    return false;
  if (!isJoined())
    return false;
  if (port < 1 || port > 223)
    return false;

  if (data == nullptr)
    return false;

  // AT+SEND=<Port>:<Ack>:<Payload>
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "AT+SEND=%d:%d:%s", port, confirmed ? 1 : 0, data);

  // Clear previous flags
  _controller->clearEvents(LSM_EVT_TX_SUCCESS | LSM_EVT_TX_FAIL | LSM_EVT_RX_TIMEOUT);

  LSM_LOG_INFO("LORA", "Sending LoRaWAN package (Port: %d, Confirmed: %d)", port, confirmed);
  LSM_LOG_VERBOSE("LORA", "Payload: %s", data);

  // Send the payload (this responds OK quickly if well-formatted and not busy)
  AtError err = _controller->sendCommand(cmd, 3000, 1);
  if (err != AtError::OK) {
    LSM_LOG_ERROR("LORA", "Failed to dispatch LoRaWAN transmission request.");
    return false;
  }

  // If timeoutMs is 0 (Auto), calculate dynamically based on retries
  uint32_t calcTimeoutMs = timeoutMs;

  if (confirmed) {
    int retries = -1;
    if (_cachedConfirmRetry >= 0)
      retries = _cachedConfirmRetry;
    else
      retries = getConfirmRetry();
    if (retries <= 0)
      retries = 3; // Fallback

    // Confirmed Retries = 1 initial + N retries. So total transmits = retries + 1.
    // Each transmit yields 2 RX_TIMEOUTs if no ACK is received.
    int expectedTimeouts = (retries + 1) * 2;
    int receivedTimeouts = 0;

    // Base safety 5s + each attempt takes ~4.5 seconds of radio (TX + Rx1 + Rx2 + MAC delay)
    uint32_t dynamicTimeoutMs = 5000 + ((retries + 1) * 4500);
    if (calcTimeoutMs == 0 || calcTimeoutMs < dynamicTimeoutMs) {
      calcTimeoutMs = dynamicTimeoutMs;
    }

    uint32_t startMs = (uint32_t)(esp_timer_get_time() / 1000);
    _controller->clearEvents(LSM_EVT_RX_TIMEOUT);

    while (((uint32_t)(esp_timer_get_time() / 1000) - startMs) < calcTimeoutMs) {
      uint32_t expectedBits = LSM_EVT_TX_SUCCESS | LSM_EVT_TX_FAIL | LSM_EVT_RX_TIMEOUT;
      if (_linkCheckRequested)
        expectedBits |= LSM_EVT_LINK_CHECK_ANS;

      // Wait iteratively for events
      uint32_t result = _controller->waitForEvent(expectedBits, 3000, true);

      if (result & LSM_EVT_LINK_CHECK_ANS) {
        _linkCheckRequested = false;
        return true;
      }
      if (result & LSM_EVT_TX_SUCCESS) {
        if (!_linkCheckRequested)
          return true; // Downlink intercepted or quick success
      }
      if (result & LSM_EVT_TX_FAIL) {
        _linkCheckRequested = false;
        return false;
      }
      if (result & LSM_EVT_RX_TIMEOUT) {
        receivedTimeouts++;
        if (receivedTimeouts >= expectedTimeouts) {
          if (_linkCheckRequested) {
            _linkCheckRequested = false;
          }
          return false; // MAC layer exhausted retries gracefully. NO recoverModule().
        }
      }
    }

    // If we reach a total asynchronous timeout, it means the module hung (no output)
    LSM_LOG_ERROR("LORA", "MAC transmission timeout: Module hung without outputting result.");
    _controller->recoverModule();
    return false; // Real MAC layer error (no UART output)
  }
  else {
    // For pure Unconfirmed, we wait for `MAC rxTimeOut` (2 per transmission attempt)
    int retries = -1;
    if (_cachedUnconfirmRetry >= 0)
      retries = _cachedUnconfirmRetry;
    else
      retries = getUnconfirmRetry();
    if (retries < 0)
      retries = 5; // Fallback if error occurred

    // (N retries) * 2 RX windows
    int expectedTimeouts = retries * 2;
    int receivedTimeouts = 0;

    // As retries can take time, extend the base timeout for dynamic safety
    uint32_t dynamicTimeoutMs = 5000 + (retries * 3500);
    if (calcTimeoutMs == 0 || calcTimeoutMs < dynamicTimeoutMs) {
      calcTimeoutMs = dynamicTimeoutMs;
    }

    uint32_t startMs = (uint32_t)(esp_timer_get_time() / 1000);
    _controller->clearEvents(LSM_EVT_RX_TIMEOUT); // clear before starting

    while (((uint32_t)(esp_timer_get_time() / 1000) - startMs) < calcTimeoutMs) {
      uint32_t expectedBits = LSM_EVT_TX_SUCCESS | LSM_EVT_RX_TIMEOUT;
      if (_linkCheckRequested)
        expectedBits |= LSM_EVT_LINK_CHECK_ANS;

      // Wait for a MAC RX timeout, or if magically a SUCCESS arrives (e.g., LinkCheck piggyback)
      uint32_t result = _controller->waitForEvent(expectedBits, 3000, true);

      if (result & LSM_EVT_LINK_CHECK_ANS) {
        _linkCheckRequested = false;
        return true; // Total success: metadata received
      }

      if (result & LSM_EVT_TX_SUCCESS) {
        if (!_linkCheckRequested)
          return true; // Downlink intercepted or MAC finished early
      }

      if (result & LSM_EVT_RX_TIMEOUT) {
        receivedTimeouts++;
        if (receivedTimeouts >= expectedTimeouts) {
          if (_linkCheckRequested) {
            _linkCheckRequested = false;
            _controller->recoverModule();
            return false;
          }
          return true;
        }
      }
    }

    // If we reach a total asynchronous timeout, it means the module hung (no output)
    LSM_LOG_ERROR("LORA", "MAC transmission timeout: Module hung without outputting result.");
    _controller->recoverModule();
    return false; // Global wait timeout
  }
}

bool LSM1x0A_LoRaWAN::requestLinkCheck()
{
  if (!_controller)
    return false;
  if (!isJoined())
    return false;

  // AT+LINKC does not generate an immediate event, it only schedules the request for the next Uplink
  LSM_LOG_INFO("LORA", "Requesting LinkCheck for the next uplink.");
  AtError err = _controller->sendCommand(LsmAtCommand::LINK_CHECK, 2000, 1);
  if (err == AtError::OK) {
    _linkCheckRequested = true;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::isJoined() const
{
  return _isJoined;
}

void LSM1x0A_LoRaWAN::setJoined(bool joined)
{
  _isJoined = joined;
}

bool LSM1x0A_LoRaWAN::recoverConnection(int maxRetries)
{
  for (int i = 0; i < maxRetries; i++) {
    // Here we assume OTAA by default, although in the future the mode could be cached
    if (join(_joinMode)) {
      return true;
    }
    vTaskDelay(pdMS_TO_TICKS(2000)); // Pause between attempts
  }
  return false;
}
