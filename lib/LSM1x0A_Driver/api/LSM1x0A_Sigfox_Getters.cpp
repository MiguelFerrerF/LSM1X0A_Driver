#include "../LSM1x0A_Controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Local helper to parse a hex string with optional separators (like ':') into a byte array.
static size_t parseHexStringToArray(const char* hexString, uint8_t* outArray, size_t arraySize)
{
  if (!hexString || !outArray || arraySize == 0)
    return 0;

  size_t count = 0;
  const char* ptr = hexString;
  while (*ptr && count < arraySize) {
    if (*ptr == ':' || *ptr == '-' || *ptr == ' ') {
      ptr++;
      continue;
    }
    if (isxdigit((unsigned char)ptr[0]) && isxdigit((unsigned char)ptr[1])) {
      char byteStr[3] = {ptr[0], ptr[1], '\0'};
      outArray[count++] = (uint8_t)strtol(byteStr, NULL, 16);
      ptr += 2;
    } else {
      break; 
    }
  }
  return count;
}

bool LSM1x0A_Sigfox::getDeviceID(char* outBuffer, size_t maxLen)
{
  if (!outBuffer || maxLen < 1)
    return false;

  if (_cachedDeviceID[0] != '\0') {
    strncpy(outBuffer, _cachedDeviceID, maxLen - 1);
    outBuffer[maxLen - 1] = '\0';
    return true;
  }

  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::DEV_ID);

  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 3000) == AtError::OK) {
    strncpy(_cachedDeviceID, buf, sizeof(_cachedDeviceID) - 1);
    _cachedDeviceID[sizeof(_cachedDeviceID) - 1] = '\0';
    strncpy(outBuffer, _cachedDeviceID, maxLen - 1);
    outBuffer[maxLen - 1] = '\0';
    return true;
  }
  return false;
}

bool LSM1x0A_Sigfox::getDeviceID(uint8_t* outArray, size_t arraySize)
{
  char rx[32] = {0};
  if (getDeviceID(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
  }
  return false;
}

bool LSM1x0A_Sigfox::getInitialPAC(char* outBuffer, size_t maxLen)
{
  if (!outBuffer || maxLen < 1)
    return false;

  if (_cachedPAC[0] != '\0') {
    strncpy(outBuffer, _cachedPAC, maxLen - 1);
    outBuffer[maxLen - 1] = '\0';
    return true;
  }

  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::DEV_PAC);

  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 3000) == AtError::OK) {
    strncpy(_cachedPAC, buf, sizeof(_cachedPAC) - 1);
    _cachedPAC[sizeof(_cachedPAC) - 1] = '\0';
    strncpy(outBuffer, _cachedPAC, maxLen - 1);
    outBuffer[maxLen - 1] = '\0';
    return true;
  }
  return false;
}

bool LSM1x0A_Sigfox::getInitialPAC(uint8_t* outArray, size_t arraySize)
{
  char rx[32] = {0};
  if (getInitialPAC(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
  }
  return false;
}

LsmRCChannel LSM1x0A_Sigfox::getRcChannel()
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::RC_CHANNEL);
  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 2000) == AtError::OK) {
    int rcVal = atoi(buf);
    if (rcVal >= 0 && rcVal <= (int)LsmRCChannel::RC_UNKNOWN) {
      _cachedRcChannel = (LsmRCChannel)rcVal;
      return _cachedRcChannel;
    }
  }
  return LsmRCChannel::RC_UNKNOWN;
}

int LSM1x0A_Sigfox::getRadioPower()
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::RADIO_POWER);
  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 2000) == AtError::OK) {
    _cachedRadioPower = atoi(buf);
    return _cachedRadioPower;
  }
  return -1; // Unknown or error
}

bool LSM1x0A_Sigfox::getPublicKeyMode()
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::ENCRYPT_KEY);
  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 2000) == AtError::OK) {
    _cachedPublicKey = atoi(buf);
    return _cachedPublicKey == 1;
  }
  return false;
}

bool LSM1x0A_Sigfox::getPayloadEncryption()
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::ENCRYPT_PAYLOAD);
  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 2000) == AtError::OK) {
    _cachedEncrypt = atoi(buf);
    return _cachedEncrypt == 1;
  }
  return false;
}

int16_t LSM1x0A_Sigfox::getLastRxRSSI() const
{
  return _lastRxRSSI;
}

bool LSM1x0A_Sigfox::getLastDownlinkTime(struct tm* timeinfo) const
{
  if (!timeinfo || _lastDownlinkTime == 0)
    return false;

  struct tm* info = gmtime(&_lastDownlinkTime);
  if (info) {
    memcpy(timeinfo, info, sizeof(struct tm));
    return true;
  }
  return false;
}
