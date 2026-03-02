#include "../LSM1x0A_Controller.h"
#include "LSM1x0A_Sigfox.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
