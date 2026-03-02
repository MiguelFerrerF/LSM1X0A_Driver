#include "LSM1x0A_Sigfox.h"
#include "../LSM1x0A_Controller.h"
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
  } else {
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::SEND_BIT, bit ? 1 : 0);
  }
  
  if (downlink) {
    char rxBuffer[32] = {0};
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      return true;
    }
    // En Sigfox el downlink suele perderse con más facilidad, o el timeout ocurre.
    // Retornamos false para indicar que NO hubo downlink, pero la red sí consumió la solicitud OOB/Uplink.
    return false;
  }

  // Use a longer timeout (30s) because Sigfox TX window takes some time
  return _controller->sendCommand(cmd, 30000) == AtError::OK;
}

bool LSM1x0A_Sigfox::sendFrame(const char* text, bool downlink, uint8_t txRepeat)
{
  char cmd[64];
  if (downlink || txRepeat != 2) {
    snprintf(cmd, sizeof(cmd), "%s%s,%d,%d", LsmAtCommand::SEND_FRAME, text, downlink ? 1 : 0, txRepeat);
  } else {
    snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::SEND_FRAME, text);
  }

  if (downlink) {
    char rxBuffer[32] = {0};
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      return true;
    }
    return false;
  }

  return _controller->sendCommand(cmd, 30000) == AtError::OK;
}

bool LSM1x0A_Sigfox::sendHexFrame(const uint8_t* payload, size_t len, bool downlink, uint8_t txRepeat)
{
  if (len > 12) return false;

  char hexStr[25];  // 12 bytes * 2 + null terminator
  for (size_t i = 0; i < len; i++) {
    // Arduino's sprintf doesn't have secure alternatives but we know size is safe here
    sprintf(hexStr + (i * 2), "%02X", payload[i]);
  }

  char cmd[64];
  if (downlink || txRepeat != 2) {
    snprintf(cmd, sizeof(cmd), "%s%s,%d,%d", LsmAtCommand::SEND_HEX, hexStr, downlink ? 1 : 0, txRepeat);
  } else {
    snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::SEND_HEX, hexStr);
  }

  if (downlink) {
    char rxBuffer[32] = {0};
    AtError err = _controller->sendCommandWithResponse(cmd, rxBuffer, sizeof(rxBuffer), "+RX_H=", 60000);
    if (err == AtError::OK) {
      parseSigfoxDownlink(rxBuffer);
      return true;
    }
    return false;
  }

  return _controller->sendCommand(cmd, 30000) == AtError::OK;
}

bool LSM1x0A_Sigfox::sendOutOfBand()
{
  // ATS300 Out of band message. Usually takes standard TX time (approx 6 seconds).
  return _controller->sendCommand(LsmAtCommand::SEND_OOB, 10000) == AtError::OK;
}

bool LSM1x0A_Sigfox::parseSigfoxDownlink(const char* rxBuffer)
{
  if (!rxBuffer || strlen(rxBuffer) < 12) return false;

  char hexTemp[9];
  // Parsear TIMESTAMP (Primeros 8 caracteres)
  memcpy(hexTemp, rxBuffer, 8);
  hexTemp[8]  = '\0';
  uint32_t ts = strtoul(hexTemp, NULL, 16);

  // Ajustar el reloj interno del ESP32 con esta hora
  time_t         rawTime = (time_t)ts;
  struct timeval tv      = {.tv_sec = static_cast<long>(rawTime), .tv_usec = 0};
  settimeofday(&tv, NULL);

  _lastDownlinkTime = rawTime;

  // Parsear RSSI (Siguientes 4 caracteres)
  char rssiTemp[5];
  memcpy(rssiTemp, rxBuffer + 8, 4);
  rssiTemp[4]  = '\0';
  uint32_t rawRssi = strtoul(rssiTemp, NULL, 16);
  _lastRxRSSI      = (int16_t)rawRssi;

  return true;
}

bool LSM1x0A_Sigfox::join()
{
  return sendBit(true, true, 1);
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
    } else {
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
  if (mode < 0 || mode > 12) return false;
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
