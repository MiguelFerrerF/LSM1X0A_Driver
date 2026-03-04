#include "LSM1x0A_AtParser.h"

LSM1x0A_AtParser::LSM1x0A_AtParser()
{
  _syncSem = xSemaphoreCreateBinary();
}

LSM1x0A_AtParser::~LSM1x0A_AtParser()
{
  if (_syncSem != nullptr) {
    vSemaphoreDelete(_syncSem);
    _syncSem = nullptr;
  }
}

bool LSM1x0A_AtParser::init(UartDriver* driver, AtEventCallback onEvent, void* eventCtx)
{
  _driver         = driver;
  _eventCallback  = onEvent;
  _eventCtx       = eventCtx;
  _lineIdx        = 0;
  _pendingCommand = false;

  // Initialize the UART driver with the static callback
  return _driver->init(UART_NUM_2, LSM1X0A_BAUDRATE, LSM1X0A_RX_PIN, LSM1X0A_TX_PIN, LSM1x0A_AtParser::staticRxCallback, this);
}

void LSM1x0A_AtParser::staticRxCallback(void* ctx, uint8_t* data, size_t len)
{
  // Secure context pointer and call the instance method to process the received data
  ((LSM1x0A_AtParser*)ctx)->eatBuffer(data, len);
}

// Based on lora_command.c and lora_at.h
AtError LSM1x0A_AtParser::parseErrorString(const char* line)
{
  // Use stricmp/strcmp instead of strstr to avoid false positives
  if (strcmp(line, "OK") == 0)
    return AtError::OK;
  if (strcmp(line, "AT_ERROR") == 0)
    return AtError::GENERIC_ERROR;
  if (strcmp(line, "AT_PARAM_ERROR") == 0)
    return AtError::PARAM_ERROR;
  if (strcmp(line, "AT_BUSY_ERROR") == 0)
    return AtError::BUSY;
  if (strcmp(line, "AT_TEST_PARAM_OVERFLOW") == 0)
    return AtError::TEST_PARAM_OVERFLOW;
  if (strcmp(line, "AT_NO_NETWORK_JOINED") == 0)
    return AtError::NO_NET_JOINED;
  if (strcmp(line, "AT_RX_ERROR") == 0)
    return AtError::RX_ERROR;
  if (strcmp(line, "AT_NO_CLASS_B_ENABLED") == 0)
    return AtError::NO_CLASS_B_ENABLE;
  if (strcmp(line, "AT_DUTYCYCLE_RESTRICTED") == 0)
    return AtError::DUTY_CYCLE_RESTRICT;
  if (strcmp(line, "AT_CRYPTO_ERROR") == 0)
    return AtError::CRYPTO_ERROR;
  if (strcmp(line, "AT_LIB_ERROR") == 0)
    return AtError::LIBRARY_ERROR;
  if (strcmp(line, "AT_TX_TIMEOUT") == 0)
    return AtError::TX_TIMEOUT;
  if (strcmp(line, "AT_RX_TIMEOUT") == 0)
    return AtError::RX_TIMEOUT;
  if (strcmp(line, "AT_RECONF_ERROR") == 0)
    return AtError::RECONF_ERROR;
  if (strcmp(line, "BOOTALERT") == 0)
    return AtError::BOOT_ALERT;

  return AtError::UNKNOWN;
}

void LSM1x0A_AtParser::eatBuffer(const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    char c = (char)data[i];
    // Standard end-of-line detection \r\n
    if (c == '\n' || c == '\r') {
      if (_lineIdx > 0) {
        _lineBuffer[_lineIdx] = '\0'; // Null-terminate
        processLine(_lineBuffer);
        _lineIdx = 0; // Reset for next line
      }
    }
    else {
      // Protection against internal buffer overflow
      if (_lineIdx < AT_BUFFER_SIZE - 1) {
        _lineBuffer[_lineIdx++] = c;
      }
      // If full, ignore the rest of the line until \n
    }
  }
}

// -----------------------------------------------------------
// MAIN EVENT PARSER
// -----------------------------------------------------------
void LSM1x0A_AtParser::processLine(char* line)
{
  // 1. LIMPIEZA
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n'))
    line[--len] = '\0';
  if (len == 0)
    return;

  LSM_LOG_VERBOSE("UART", "RX Line: %s", line);

  // 2. DETECTION OF ASYNCHRONOUS EVENTS (+EVT)
  // Based on the functions: AT_event_join, AT_event_receive, AT_event_confirm
  char* evtPtr = strstr(line, "+EVT:");

  if (evtPtr != NULL) {
    char* payload = evtPtr + 5; // Skip the "+EVT:" prefix
    if (_eventCallback) {
      // --- A. JOIN ---
      if (strncmp(payload, "JOINED", 6) == 0) {
        _eventCallback(LsmEvent::JOIN, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "JOIN FAILED", 11) == 0) {
        _eventCallback(LsmEvent::JOIN, "FAILED", _eventCtx);
      }

      // --- B. DATA RECEPTION (RECV) ---
      // Format: RECV_CONFIRMED:PORT:SIZE:DATA...
      else if (strncmp(payload, "RECV_", 5) == 0) {
        // Search for the first ':' to skip CONFIRMED/UNCONFIRMED
        char* ptr = strchr(payload, ':');
        if (ptr) {
          // Pass "PORT:SIZE:DATA" clean to the user
          _eventCallback(LsmEvent::RX_DATA, ptr + 1, _eventCtx);
        }
      }

      // --- C. RECEPTION METADATA (RX_) ---
      // Format: RX_1, PORT 2, DR 5, RSSI -90, SNR 10...
      // NOTE: Do not confuse with RECV_. We check RX_
      else if (strncmp(payload, "RX_", 3) == 0) {
        // Pass the entire metadata string for the helper to parse
        _eventCallback(LsmEvent::RX_META, payload, _eventCtx);
      }

      // --- D. SEND CONFIRMATION (TX) ---
      else if (strncmp(payload, "SEND_CONFIRMED", 14) == 0) {
        _eventCallback(LsmEvent::TX, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "SEND_FAILED", 11) == 0) {
        _eventCallback(LsmEvent::TX, "FAILED", _eventCtx);
      }

      // --- E. CLASS CHANGE ---
      else if (strncmp(payload, "SWITCH_TO_CLASS_", 16) == 0) {
        // Payload will be "SWITCH_TO_CLASS_C", we pass "C"
        _eventCallback(LsmEvent::CLASS, payload + 16, _eventCtx);
      }

      // --- F. BEACONS ---
      else if (strncmp(payload, "BEACON_LOST", 11) == 0) {
        _eventCallback(LsmEvent::BEACON, "LOST", _eventCtx);
      }
      else if (strncmp(payload, "RX_BC", 5) == 0) {
        // Beacon received with info
        _eventCallback(LsmEvent::BEACON, payload, _eventCtx);
      }
    }
    return; // Event processed, do not continue searching for command responses
  }

  // 3. SPECIAL CASES WITHOUT +EVT PREFIX
  if (strncmp(line, "NVM DATA", 8) == 0) {
    if (_eventCallback)
      _eventCallback(LsmEvent::NVM, line, _eventCtx);
    return;
  }

  if (strncmp(line, "channel_mask[", 13) == 0) {
    if (_eventCallback)
      _eventCallback(LsmEvent::CHMASK, line, _eventCtx);
    // IMPORTANT: Do not return; if it is a synchronous command waiting for the final 'OK'.
    // This will send the asynchronous event and then the Parser will continue running waiting for the OK.
  }

  // 4. Ignore irrelevant lines
  // Discard asynchronous echoes of the command itself (e.g., "AT+BAT=?")
  if (strncmp(line, "AT+", 3) == 0)
    return;

  // If we reach here, the line did NOT have +EVT, so if it starts with "confirmed flag",
  // it is isolated garbage and we discard it.
  if (strncmp(line, "confirmed flag:", 15) == 0)
    return;
  if (strncmp(line, "---", 3) == 0)
    return; // Boot logs
  if (strncmp(line, "+EVT:Prepare Frame", 18) == 0)
    return;

  // Device type detection
  if (strstr(line, "LSM100A") != nullptr)
    _deviceType = LsmModuleType::LSM100A;
  else if (strstr(line, "LSM110A") != nullptr)
    _deviceType = LsmModuleType::LSM110A;

  // Operating mode detection
  if (strstr(line, "LoRa") != nullptr)
    _detectedMode = LsmMode::LORAWAN;
  else if (strstr(line, "Sigfox") != nullptr)
    _detectedMode = LsmMode::SIGFOX;

  // Intercept MAC rxTimeOut explicitly without failing the pendingCommand
  if (strstr(line, "MAC rxTimeOut") != nullptr) {
    if (_eventCallback)
      _eventCallback(LsmEvent::RX_TIMEOUT, "TIMEOUT", _eventCtx);
  }

  if (_pendingCommand) {
    AtError err = parseErrorString(line);

    // If the line is a final return code (OK, ERROR, etc... we finish)
    if (err != AtError::UNKNOWN) {
      _lastResultError = err;
      xSemaphoreGive(_syncSem);
      return;
    }

    // Data capture (Getters). Only if the line was NOT an OK, Error, or Echo
    if (_userOutBuffer != nullptr) {
      const char* dataStart = line;
      bool        match     = (_expectedTag == nullptr); // If no tag, take the whole line

      if (_expectedTag && strstr(line, _expectedTag)) {
        dataStart = strstr(line, _expectedTag) + strlen(_expectedTag);
        match     = true;
      }

      if (match) {
        while (*dataStart == ' ' || *dataStart == ':')
          dataStart++; // Clean prefixes
        strncpy(_userOutBuffer, dataStart, _userOutSize - 1);
        _userOutBuffer[_userOutSize - 1] = '\0';
      }
    }
  }
}

// -----------------------------------------------------------
// HELPER TO PARSE COVERAGE METADATA
// -----------------------------------------------------------
bool LSM1x0A_AtParser::parseRxMetadata(const char* payload, LsmRxMetadata* out)
{
  // Example: "RX_1, PORT 2, DR 5, RSSI -90, SNR 10, DMODM 10, GWN 2"
  if (!payload || !out)
    return false;

  // Initialize defaults
  memset(out, 0, sizeof(LsmRxMetadata));
  out->hasLinkCheck = false;

  // 1. Extract Slot (RX_1, RX_C, etc)
  // The payload starts with "RX_"
  const char* pStart = payload + 3; // Skip "RX_"
  const char* pComma = strchr(pStart, ',');
  if (pComma) {
    int len = pComma - pStart;
    if (len < sizeof(out->slot)) {
      strncpy(out->slot, pStart, len);
      out->slot[len] = '\0';
    }
  }
  else
    return false; // Invalid format

  // 2. Extract standard numeric fields
  // Look for key substrings
  const char* pPort = strstr(payload, "PORT ");
  const char* pDr   = strstr(payload, "DR ");
  const char* pRssi = strstr(payload, "RSSI ");
  const char* pSnr  = strstr(payload, "SNR ");

  if (pPort)
    out->port = atoi(pPort + 5);
  if (pDr)
    out->dataRate = atoi(pDr + 3);
  if (pRssi)
    out->rssi = atoi(pRssi + 5);
  if (pSnr)
    out->snr = atoi(pSnr + 4);

  // 3. Extract LinkCheck
  const char* pDmodm = strstr(payload, "DMODM ");
  const char* pGwn   = strstr(payload, "GWN ");

  if (pDmodm && pGwn) {
    out->hasLinkCheck = true;
    out->demodMargin  = atoi(pDmodm + 6);
    out->nbGateways   = atoi(pGwn + 4);
  }

  return true;
}

bool LSM1x0A_AtParser::wakeUp(uint8_t retries, uint32_t delayMs)
{
  _driver->flushRx();
  for (uint8_t i = 0; i < retries; i++) {
    // Send Ping while silencing payload/buffer errors to avoid overwriting readings.
    AtError err = sendCommand("AT", delayMs);
    if (err == AtError::OK) {
      return true;
    }
  }
  return false;
}

AtError LSM1x0A_AtParser::sendCommand(const char* cmd, uint32_t timeoutMs)
{
  return sendCommandWithResponse(cmd, nullptr, nullptr, 0, timeoutMs);
}

AtError LSM1x0A_AtParser::waitForEvent(uint32_t timeoutMs)
{
  xSemaphoreTake(_syncSem, 0); // Clear any previous state

  if (_pendingCommand) {
    return AtError::BUSY;
  }

  _pendingCommand  = true;
  _lastResultError = AtError::TIMEOUT;

  if (xSemaphoreTake(_syncSem, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
    _pendingCommand = false;
    return AtError::TIMEOUT;
  }

  _pendingCommand = false;
  return _lastResultError;
}

AtError LSM1x0A_AtParser::sendCommandWithResponse(const char* cmd, const char* expectedTag, char* outBuffer, size_t outSize, uint32_t timeoutMs)
{
  // 1. Ensure no other transaction is in progress
  xSemaphoreTake(_syncSem, 0);

  if (_pendingCommand) {
    return AtError::BUSY;
  }

  // 2. Configure transaction context
  _pendingCommand  = true;
  _lastResultError = AtError::TIMEOUT;
  _expectedTag     = expectedTag;
  _userOutBuffer   = outBuffer; // Pointer to user's stack/global buffer
  _userOutSize     = outSize;

  // Clear user buffer for safety
  if (outBuffer)
    outBuffer[0] = '\0';

  LSM_LOG_VERBOSE("UART", "TX: %s", cmd);

  // 3. Send Command via UART
  _driver->sendData(cmd, strlen(cmd));
  _driver->sendData("\r\n", 2);

  // 4. Block waiting for response
  if (xSemaphoreTake(_syncSem, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
    LSM_LOG_DEBUG("UART", "sendCommandWithResponse timeout");
    _pendingCommand = false;
    return AtError::TIMEOUT;
  }

  _pendingCommand = false;
  _userOutBuffer  = nullptr; // Unlink buffer for safety

  return _lastResultError;
}

const char* LSM1x0A_AtParser::atErrorToString(AtError err)
{
  if (err == AtError::OK)
    return "OK";
  if (err == AtError::GENERIC_ERROR)
    return "AT_ERROR";
  if (err == AtError::PARAM_ERROR)
    return "AT_PARAM_ERROR";
  if (err == AtError::BUSY)
    return "AT_BUSY_ERROR";
  if (err == AtError::TEST_PARAM_OVERFLOW)
    return "AT_TEST_PARAM_OVERFLOW";
  if (err == AtError::NO_NET_JOINED)
    return "AT_NO_NETWORK_JOINED";
  if (err == AtError::RX_ERROR)
    return "AT_RX_ERROR";
  if (err == AtError::NO_CLASS_B_ENABLE)
    return "AT_NO_CLASS_B_ENABLED";
  if (err == AtError::DUTY_CYCLE_RESTRICT)
    return "AT_DUTYCYCLE_RESTRICTED";
  if (err == AtError::CRYPTO_ERROR)
    return "AT_CRYPTO_ERROR";
  if (err == AtError::LIBRARY_ERROR)
    return "AT_LIB_ERROR";
  if (err == AtError::TX_TIMEOUT)
    return "AT_TX_TIMEOUT";
  if (err == AtError::RX_TIMEOUT)
    return "AT_RX_TIMEOUT";
  if (err == AtError::RECONF_ERROR)
    return "AT_RECONF_ERROR";
  if (err == AtError::BOOT_ALERT)
    return "BOOTALERT";
  if (err == AtError::TIMEOUT)
    return "TIMEOUT";
  if (err == AtError::BUFFER_OVERFLOW)
    return "BUFFER_OVERFLOW";
  return "UNKNOWN";
}