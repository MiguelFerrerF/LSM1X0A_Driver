#include "LSM1x0A_Sigfox.h"
#include "../LSM1x0A_Controller.h"

bool LSM1x0A_Sigfox::setRcChannel(LsmRCChannel rc)
{
  if (rc == LsmRCChannel::RC_UNKNOWN) return false;

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RC_CHANNEL, (int)rc);
  if (_controller->sendCommand(cmd, 2000) == AtError::OK) {
    _cachedRcChannel = rc;
    return true;
  }
  return false;
}

bool LSM1x0A_Sigfox::setRadioPower(int power_dBm)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RADIO_POWER, power_dBm);
  if (_controller->sendCommand(cmd, 2000) == AtError::OK) {
    _cachedRadioPower = power_dBm;
    return true;
  }
  return false;
}

bool LSM1x0A_Sigfox::setPublicKeyMode(bool public_key)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ENCRYPT_KEY, public_key ? 1 : 0);
  if (_controller->sendCommand(cmd, 2000) == AtError::OK) {
    _cachedPublicKey = public_key ? 1 : 0;
    return true;
  }
  return false;
}

bool LSM1x0A_Sigfox::setPayloadEncryption(bool active)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ENCRYPT_PAYLOAD, active ? 1 : 0);
  if (_controller->sendCommand(cmd, 2000) == AtError::OK) {
    _cachedEncrypt = active ? 1 : 0;
    return true;
  }
  return false;
}
