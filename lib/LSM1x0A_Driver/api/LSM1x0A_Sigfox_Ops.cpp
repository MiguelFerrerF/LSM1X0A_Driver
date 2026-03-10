#include "../LSM1x0A_Controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

LSM1x0A_Sigfox::LSM1x0A_Sigfox(LSM1x0A_Controller* controller) : _controller(controller)
{
  clearCache();
}

void LSM1x0A_Sigfox::clearCache()
{
  _cachedDeviceID[0] = '\0';
  _cachedPAC[0]      = '\0';
  _cachedRcChannel   = LsmRCChannel::RC_UNKNOWN;
  _cachedRadioPower  = -1;
  _cachedPublicKey   = -1;
  _cachedEncrypt     = -1;
}

bool LSM1x0A_Sigfox::loadConfigFromModule()
{
  clearCache();

  bool success = true;
  success &= (getRcChannel() != LsmRCChannel::RC_UNKNOWN);
  success &= (getRadioPower() != -1);
  getPublicKeyMode();
  getPayloadEncryption();

  return success;
}

bool LSM1x0A_Sigfox::restoreConfig()
{
  bool success = true;
  if (_cachedRcChannel != LsmRCChannel::RC_UNKNOWN) {
    success &= setRcChannel(_cachedRcChannel);
  }
  if (_cachedRadioPower > 0) {
    success &= setRadioPower(_cachedRadioPower);
  }
  if (_cachedPublicKey != -1) {
    success &= setPublicKeyMode(_cachedPublicKey == 1);
  }
  if (_cachedEncrypt != -1) {
    success &= setPayloadEncryption(_cachedEncrypt == 1);
  }

  return success;
}

bool LSM1x0A_Sigfox::sendBit(bool bit, bool downlink, uint8_t txRepeat)
{
  char cmd[32];
  if (downlink || txRepeat != 2) {
    snprintf(cmd, sizeof(cmd), "%s%d,%d,%d", LsmAtCommand::SEND_BIT, bit ? 1 : 0, downlink ? 1 : 0, txRepeat);
  }
  else {
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::SEND_BIT, bit ? 1 : 0);
  }

  if (downlink) {
    char rxBuffer[32] = {0};
    LSM_LOG_INFO("SIGFOX", "Sending Bit (%d) w/ Downlink requested", bit);
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      LSM_LOG_INFO("SIGFOX", "Bit TX Success - Downlink received and parsed.");
      return true;
    }
    // In Sigfox, the downlink is often lost more easily, or the timeout occurs.
    // We return false to indicate that there was NO downlink, but the network did consume the OOB/Uplink request.
    LSM_LOG_WARN("SIGFOX", "Bit TX executed but Downlink was not successfully received.");
    return false;
  }

  // Use a longer timeout (30s) because Sigfox TX window takes some time
  LSM_LOG_INFO("SIGFOX", "Sending Bit (%d) without Downlink", bit);
  bool success = _controller->sendCommand(cmd, 30000) == AtError::OK;
  if (!success) {
    LSM_LOG_ERROR("SIGFOX", "Failed to send Bit.");
  }
  return success;
}

bool LSM1x0A_Sigfox::sendString(const char* text, bool downlink, uint8_t txRepeat)
{
  if (!isValidAscii(text, 12))
    return false;

  size_t length = strlen(text);
  char   cmd[64];
  if (downlink || txRepeat != 2) {
    snprintf(cmd, sizeof(cmd), "%s%zu,%s,%d,%d", LsmAtCommand::SEND_STRING, length, text, downlink ? 1 : 0, txRepeat);
  }
  else {
    snprintf(cmd, sizeof(cmd), "%s%zu,%s", LsmAtCommand::SEND_STRING, length, text);
  }

  if (downlink) {
    char rxBuffer[32] = {0};
    LSM_LOG_INFO("SIGFOX", "Sending String (%s) w/ Downlink requested", text);
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      LSM_LOG_INFO("SIGFOX", "String TX Success - Downlink received.");
      return true;
    }
    LSM_LOG_WARN("SIGFOX", "String TX executed but Downlink was not successfully received.");
    return false;
  }

  LSM_LOG_INFO("SIGFOX", "Sending String (%s) without Downlink", text);
  bool success = _controller->sendCommand(cmd, 30000) == AtError::OK;
  if (!success) {
    LSM_LOG_ERROR("SIGFOX", "Failed to send String.");
  }
  return success;
}

bool LSM1x0A_Sigfox::sendPayload(const uint8_t* payload, size_t len, bool downlink, uint8_t txRepeat)
{
  if (len > 12)
    return false;

  char hexStr[25]; // 12 bytes * 2 + null terminator
  for (size_t i = 0; i < len; i++) {
    sprintf(hexStr + (i * 2), "%02X", payload[i]);
  }

  char cmd[64];
  if (downlink || txRepeat != 2) {
    snprintf(cmd, sizeof(cmd), "%s%s,%d,%d", LsmAtCommand::SEND_PAYLOAD, hexStr, downlink ? 1 : 0, txRepeat);
  }
  else {
    snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::SEND_PAYLOAD, hexStr);
  }

  if (downlink) {
    char rxBuffer[32] = {0};
    LSM_LOG_INFO("SIGFOX", "Sending Hex Payload (%s) w/ Downlink requested", hexStr);
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      LSM_LOG_INFO("SIGFOX", "Payload TX Success - Downlink received.");
      return true;
    }
    LSM_LOG_WARN("SIGFOX", "Payload TX executed but Downlink was not successfully received.");
    return false;
  }

  LSM_LOG_INFO("SIGFOX", "Sending Hex Payload (%s) without Downlink", hexStr);
  bool success = _controller->sendCommand(cmd, 30000) == AtError::OK;
  if (!success) {
    LSM_LOG_ERROR("SIGFOX", "Failed to send Payload.");
  }
  return success;
}

bool LSM1x0A_Sigfox::sendOutOfBand()
{
  // ATS300 Out of band message. Usually takes standard TX time (approx 6 seconds).
  return _controller->sendCommand(LsmAtCommand::SEND_OOB, 10000) == AtError::OK;
}

bool LSM1x0A_Sigfox::parseSigfoxDownlink(const char* rxBuffer)
{
  if (!rxBuffer || strlen(rxBuffer) < 12)
    return false;

  char hexTemp[9];
  // Parse TIMESTAMP (First 8 characters)
  memcpy(hexTemp, rxBuffer, 8);
  hexTemp[8]  = '\0';
  uint32_t ts = strtoul(hexTemp, NULL, 16);

  // Adjust the ESP32 internal clock with this time
  time_t         rawTime = (time_t)ts;
  struct timeval tv      = {.tv_sec = static_cast<long>(rawTime), .tv_usec = 0};
  settimeofday(&tv, NULL);

  _lastDownlinkTime = rawTime;

  // Parse RSSI (Next 4 characters)
  char rssiTemp[5];
  memcpy(rssiTemp, rxBuffer + 8, 4);
  rssiTemp[4]      = '\0';
  uint32_t rawRssi = strtoul(rssiTemp, NULL, 16);
  _lastRxRSSI      = (int16_t)rawRssi;

  // Log the parsed downlink information
  struct tm* timeInfo;
  timeInfo = gmtime(&rawTime);

  LSM_LOG_VERBOSE("SIGFOX", "Parsed Downlink - TS: %04d-%02d-%02d %02d:%02d:%02d, RSSI: %d", timeInfo->tm_year + 1900, timeInfo->tm_mon + 1,
                  timeInfo->tm_mday, timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, _lastRxRSSI);

  return true;
}

bool LSM1x0A_Sigfox::join()
{
  return sendBit(true, true, 2);
}

// =========================================================================
// Tests and Scan Modes
// =========================================================================

bool LSM1x0A_Sigfox::testContinuousWave(long freqHz)
{
  if (freqHz > 0) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "%s%ld", LsmAtCommand::TEST_CW, freqHz);
    return _controller->sendCommand(cmd, 2000) == AtError::OK;
  }
  return _controller->sendCommand(LsmAtCommand::TEST_CW, 2000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testPRBS9(long freqHz, int bitrate)
{
  if (freqHz > 0) {
    char cmd[32];
    if (bitrate > 0) {
      snprintf(cmd, sizeof(cmd), "%s%ld,%d", LsmAtCommand::TEST_PRBS9, freqHz, bitrate);
    }
    else {
      snprintf(cmd, sizeof(cmd), "%s%ld", LsmAtCommand::TEST_PRBS9, freqHz);
    }
    return _controller->sendCommand(cmd, 2000) == AtError::OK;
  }
  return _controller->sendCommand(LsmAtCommand::TEST_PRBS9, 2000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testMonarchScan(int timeoutSecs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TEST_MONARCH, timeoutSecs);
  // Give it slightly more time than the timeout
  return _controller->sendCommand(cmd, (timeoutSecs * 1000) + 1000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testMode(int mode)
{
  if (mode < 0 || mode > 12)
    return false;
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TEST_MODE, mode);
  return _controller->sendCommand(cmd, 10000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testListenLoop()
{
  return _controller->sendCommand(LsmAtCommand::TEST_RL, 5000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testSendLoop()
{
  return _controller->sendCommand(LsmAtCommand::TEST_SL, 5000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testSendP2P()
{
  return _controller->sendCommand(LsmAtCommand::TEST_SP2P, 5000) == AtError::OK;
}

bool LSM1x0A_Sigfox::testReceiveP2P()
{
  return _controller->sendCommand(LsmAtCommand::TEST_RP2P, 5000) == AtError::OK;
}

bool LSM1x0A_Sigfox::isValidHex(const char* str, size_t maxLen)
{
  if (str == nullptr) {
    return false;
  }

  size_t length = 0;
  while (*str && length < maxLen) {
    if (!((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f'))) {
      LSM_LOG_ERROR("SIGFOX", "Invalid hex character detected: '%c' at position %zu", *str, length);
      return false; // Non-hexadecimal character detected
    }
    str++;
    length++;
  }
  return true;
}

bool LSM1x0A_Sigfox::isValidAscii(const char* str, size_t maxLen)
{
  if (str == nullptr) {
    return false;
  }

  size_t length = 0;
  while (*str && length < maxLen) {
    // Acceptable range of printable ASCII characters (including space)
    if (*str < 32 || *str > 126) {
      LSM_LOG_ERROR("SIGFOX", "Invalid ASCII character detected: 0x%02X at position %zu", static_cast<unsigned char>(*str), length);
      return false; // Control or non-standard character detected
    }
    str++;
    length++;
  }

  if (length >= maxLen && *str != '\0') {
    LSM_LOG_ERROR("SIGFOX", "Input string exceeds maximum length of %zu characters", maxLen);
    return false;
  }
  return true;
}
