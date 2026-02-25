#include "../LSM1x0A_Controller.h"
#include "LSM1x0A_LoRaWAN.h"
#include <stdio.h>
#include <string.h>

// Helper auxiliar local para formatear cadenas hexadecimales añadiendo delimitadores (:)
static bool formatHexWithColons(const char* in, char* out, size_t outSize, size_t expectedBytes)
{
  if (strlen(in) != expectedBytes * 2)
    return false;
  size_t outIdx = 0;
  for (size_t i = 0; i < expectedBytes; ++i) {
    if (outIdx + DEFAULT_MAX_RETRIES > outSize)
      return false;
    out[outIdx++] = in[i * 2];
    out[outIdx++] = in[i * 2 + 1];
    if (i < expectedBytes - 1) {
      out[outIdx++] = ':';
    }
  }
  out[outIdx] = '\0';
  return true;
}

// Mácaras fijas para optimizar memoria flash y consumo (Progmem style)
static const char* const M_8CH[]  = {"000F", "00F0", "00FF"};
static const char* const M_72CH[] = {"00FF:0000:0000:0000:0000", "FF00:0000:0000:0000:0000", "0000:00FF:0000:0000:0000", "0000:FF00:0000:0000:0000",
                                     "0000:0000:00FF:0000:0000", "0000:0000:FF00:0000:0000", "0000:0000:0000:00FF:0000", "0000:0000:0000:FF00:0000",
                                     "0000:0000:0000:0000:00FF", "FFFF:FFFF:FFFF:FFFF:00FF"};
static const char* const M_96CH[] = {"00FF:0000:0000:0000:0000:0000", "FF00:0000:0000:0000:0000:0000", "0000:00FF:0000:0000:0000:0000",
                                     "0000:FF00:0000:0000:0000:0000", "0000:0000:00FF:0000:0000:0000", "0000:0000:FF00:0000:0000:0000",
                                     "0000:0000:0000:00FF:0000:0000", "0000:0000:0000:FF00:0000:0000", "0000:0000:0000:0000:00FF:0000",
                                     "0000:0000:0000:0000:FF00:0000", "0000:0000:0000:0000:0000:00FF", "0000:0000:0000:0000:0000:FF00",
                                     "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF"};

LSM1x0A_LoRaWAN::LSM1x0A_LoRaWAN(LSM1x0A_Controller* controller) : _controller(controller)
{
}

bool LSM1x0A_LoRaWAN::setDevEUI(const char* devEui)
{
  if (!devEui || strlen(devEui) < 16)
    return false;
  char formatted[32];
  if (!formatHexWithColons(devEui, formatted, sizeof(formatted), 8))
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_EUI, formatted);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    strncpy(_cachedDevEui, formatted, sizeof(_cachedDevEui));
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setAppEUI(const char* appEui)
{
  if (!appEui || strlen(appEui) < 16)
    return false;
  char formatted[32];
  if (!formatHexWithColons(appEui, formatted, sizeof(formatted), 8))
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_EUI, formatted);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setAppKey(const char* appKey)
{
  if (!appKey || strlen(appKey) < 32)
    return false;
  char formatted[64];
  if (!formatHexWithColons(appKey, formatted, sizeof(formatted), 16))
    return false;

  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_KEY, formatted);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setNwkKey(const char* nwkKey)
{
  if (!nwkKey || strlen(nwkKey) < 32)
    return false;
  char formatted[64];
  if (!formatHexWithColons(nwkKey, formatted, sizeof(formatted), 16))
    return false;

  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_KEY, formatted);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setDevAddr(const char* devAddr)
{
  if (!devAddr || strlen(devAddr) < 8)
    return false;
  char formatted[32];
  if (!formatHexWithColons(devAddr, formatted, sizeof(formatted), 4))
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_ADDR, formatted);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    strncpy(_cachedDevAddr, formatted, sizeof(_cachedDevAddr));
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setAppSKey(const char* appSKey)
{
  if (!appSKey || strlen(appSKey) < 32)
    return false;
  char formatted[64];
  if (!formatHexWithColons(appSKey, formatted, sizeof(formatted), 16))
    return false;

  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_SKEY, formatted);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setNwkSKey(const char* nwkSKey)
{
  if (!nwkSKey || strlen(nwkSKey) < 32)
    return false;
  char formatted[64];
  if (!formatHexWithColons(nwkSKey, formatted, sizeof(formatted), 16))
    return false;

  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_SKEY, formatted);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setNwkID(int nwkId)
{
  if (nwkId < 0 || nwkId > 127)
    return false;
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::NWK_ID, nwkId);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    snprintf(_cachedNwkID, sizeof(_cachedNwkID), "%d", nwkId);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setJoinMode(LsmJoinMode mode)
{
  _joinMode = mode;
  return true;
}

bool LSM1x0A_LoRaWAN::setBand(LsmBand band)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::BAND, (int)band);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedBand = band;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setClass(LsmClass lorawanClass)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%c", LsmAtCommand::CLASS, lorawanClass == LsmClass::CLASS_A ? 'A' : (lorawanClass == LsmClass::CLASS_B ? 'B' : 'C'));
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setADR(bool enabled)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ADAPTIVE_DR, enabled ? 1 : 0);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedAdrEnabled = enabled;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setDataRate(LsmDataRate dr)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::DR, (int)dr);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedDataRate = dr;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setDutyCycle(bool enabled)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::DUTY_CYCLE, enabled ? 1 : 0);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedDutyCycle = enabled;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setRx1Delay(int delayMs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX1_DELAY, delayMs);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setRx2Delay(int delayMs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX2_DELAY, delayMs);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setRx2DataRate(LsmDataRate dr)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX2_DR, (int)dr);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setRx2Frequency(int freqHz)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX2_FREQ, freqHz);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setJoin1Delay(int delayMs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN_DELAY_1, delayMs);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setJoin2Delay(int delayMs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN_DELAY_2, delayMs);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setTxPower(LsmTxPower power)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TX_POWER, (int)power);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setPingSlot(LsmPingSlot slot)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::PING_SLOT, (int)slot);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setNetworkType(LsmNetworkType type)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::NETWORK_TYPE, (int)type);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setConfirmRetry(int retries)
{
  if (retries < 1 || retries > 15)
    return false;
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::CONFIRM_RETRY, retries);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedConfirmRetry = retries;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setUnconfirmRetry(int retries)
{
  if (retries < 1 || retries > 15)
    return false;
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::UNCONFIRM_RETRY, retries);
  if (_controller->sendCommand(cmd, 1000) == AtError::OK) {
    _cachedUnconfirmRetry = retries;
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::setChannelMask(LsmBand band, int subBand)
{
  const char* const* targetMasks = nullptr;
  int                maxCount    = 0;

  switch (band) {
    case LsmBand::EU868:
    case LsmBand::AS923_1:
    case LsmBand::KR920:
    case LsmBand::IN865:
    case LsmBand::RU864:
    case LsmBand::AS923_4:
    case LsmBand::EU433:
    case LsmBand::CN779:
      targetMasks = M_8CH;
      maxCount    = 2; // last logic index is 2
      break;
    case LsmBand::US915:
    case LsmBand::AU915:
      targetMasks = M_72CH;
      maxCount    = 9; // last logic index is 9
      break;
    case LsmBand::CN470:
      targetMasks = M_96CH;
      maxCount    = 12; // last logic index is 12
      break;
    default:
      return false; // Unknown Band length
  }

  // Si no pasamos index correcto (0-N) entonces activamos mascara completa (ALL = maxCount)
  int idx = (subBand >= 0 && subBand < maxCount) ? subBand : maxCount;

  if (!targetMasks[idx] || strlen(targetMasks[idx]) < 4)
    return false;
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CHANNEL_MASK, targetMasks[idx]);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setDevNonce(int nonce)
{
  if (nonce < 0 || nonce > 255)
    return false;
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::DEVNONCE, nonce);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::resetDevNonce()
{
  return setDevNonce(0);
}

bool LSM1x0A_LoRaWAN::setRfTestConfig(long freqHz, int power, int bw, int sf_dr, int cr, int lna)
{
  char cmd[64];
  // Formato: AT+TCONF=<Freq>:<Power>:<BW>:<SF/Dr>:<CR>:<Lna>
  snprintf(cmd, sizeof(cmd), "%s%ld:%d:%d:%d:%d:%d", LsmAtCommand::TTEST_CONF, freqHz, power, bw, sf_dr, cr, lna);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startTxTest(int packets)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TTEST_TX, packets);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startRxTest(int packets)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TTEST_RX, packets);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startTxTone()
{
  return _controller->sendCommand(LsmAtCommand::TTEST_TONE, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startRxRssiTone()
{
  return _controller->sendCommand(LsmAtCommand::TTEST_RSSI, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::stopTest()
{
  return _controller->sendCommand(LsmAtCommand::TTEST_STOP, 2000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setCertificationMode(LsmJoinMode mode)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::CERTIF_MODE, mode == LsmJoinMode::OTAA ? 1 : 0);
  return _controller->sendCommand(cmd, 2000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startTxHoppingTest(long fStart, long fStop, long fDelta, int packets)
{
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%ld,%ld,%ld,%d", LsmAtCommand::TTEST_TX_HOP, fStart, fStop, fDelta, packets);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startContinuousModulationTx()
{
  return _controller->sendCommand(LsmAtCommand::TTEST_MTX, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::startContinuousModulationRx()
{
  return _controller->sendCommand(LsmAtCommand::TTEST_MRX, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::sendCertificationPacket()
{
  return _controller->sendCommand(LsmAtCommand::CERTIF_SEND, 2000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::setP2pConfig(const char* configString)
{
  if (!configString)
    return false;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::P2P_CONF, configString);
  return _controller->sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::sendP2pData(const char* hexData)
{
  if (!hexData)
    return false;
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::P2P_SEND, hexData);
  return _controller->sendCommand(cmd, 2000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::receiveP2pData(int timeoutMs)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::P2P_RECV, timeoutMs);
  return _controller->sendCommand(cmd, timeoutMs + 1000) == AtError::OK;
}

bool LSM1x0A_LoRaWAN::restoreConfig()
{
  bool success = true;

  // Restaurar identificadores si estaban en caché
  if (_cachedDevEui[0] != '\0') 
    if (!setDevEUI(_cachedDevEui)) success = false;
  if (_cachedDevAddr[0] != '\0') 
    if (!setDevAddr(_cachedDevAddr)) success = false;
  if (_cachedNwkID[0] != '\0') 
    if (!setNwkID((int)strtol(_cachedNwkID, NULL, 16))) success = false;

  // Restaurar Band y SubBand si es posible
  if (_cachedBand != LsmBand::BAND_UNKNOWN) {
    if (!setBand(_cachedBand)) success = false;
    if (_cachedSubBand != (int8_t)-1 && _cachedBand == LsmBand::US915) 
      if (!setChannelMask(_cachedBand, _cachedSubBand)) success = false;
  }

  // Restaurar configuraciones de red
  if (_cachedAdrEnabled != -1) 
    if (!setADR(_cachedAdrEnabled == 1)) success = false;
  if (_cachedDataRate != LsmDataRate::DR_UNKNOWN) 
    if (!setDataRate(_cachedDataRate)) success = false;
  if (_cachedTxPower != LsmTxPower::TP_UNKNOWN) 
    if (!setTxPower(_cachedTxPower)) success = false;
  if (_cachedDutyCycle != -1) 
    if (!setDutyCycle(_cachedDutyCycle == 1)) success = false;

  // Restaurar delays
  if (_cachedJoin1Delay != -1) 
    if (!setJoin1Delay(_cachedJoin1Delay)) success = false;
  if (_cachedJoin2Delay != -1) 
    if (!setJoin2Delay(_cachedJoin2Delay)) success = false;
  if (_cachedRx1Delay != -1) 
    if (!setRx1Delay(_cachedRx1Delay)) success = false;
  if (_cachedRx2Delay != -1) 
    if (!setRx2Delay(_cachedRx2Delay)) success = false;

  // Restaurar frecuencias / data rates de RX2
  if (_cachedRx2DataRate != LsmDataRate::DR_UNKNOWN) 
    if (!setRx2DataRate(_cachedRx2DataRate)) success = false;
  if (_cachedRx2Frequency > 0) 
    if (!setRx2Frequency(_cachedRx2Frequency)) success = false;

  // Restaurar reintentos
  if (_cachedConfirmRetry >= 0) 
    if (!setConfirmRetry(_cachedConfirmRetry)) success = false;
  if (_cachedUnconfirmRetry >= 0) 
    if (!setUnconfirmRetry(_cachedUnconfirmRetry)) success = false;

  return success;
}

bool LSM1x0A_LoRaWAN::loadConfigFromModule()
{
  bool success = true;

  // Llama a los getters internos para forzar que sus valores se sobreescriban en la caché actual
  if (!getDevEUI(_cachedDevEui, sizeof(_cachedDevEui))) success = false;
  if (!getDevAddr(_cachedDevAddr, sizeof(_cachedDevAddr))) success = false;
  
  int nwkId = getNwkID();
  if (nwkId >= 0) {
    snprintf(_cachedNwkID, sizeof(_cachedNwkID), "%08X", (unsigned int)nwkId);
  } else {
    success = false;
  }

  if (getADR() < 0) success = false;
  if (getDataRate() == LsmDataRate::DR_UNKNOWN) success = false;
  if (getTxPower() == LsmTxPower::TP_UNKNOWN) success = false;
  if (getBand() == LsmBand::BAND_UNKNOWN) success = false;
  if (getDutyCycle() < 0) success = false;
  
  if (getJoin1Delay() < 0) success = false;
  if (getJoin2Delay() < 0) success = false;
  if (getRx1Delay() < 0) success = false;
  if (getRx2Delay() < 0) success = false;
  
  if (getRx2DataRate() == LsmDataRate::DR_UNKNOWN) success = false;
  if (getRx2Frequency() < 0) success = false;

  if (getConfirmRetry() < 0) success = false;
  if (getUnconfirmRetry() < 0) success = false;

  return success;
}

void LSM1x0A_LoRaWAN::clearCache()
{
  _cachedDevEui[0]      = '\0';
  _cachedDevAddr[0]     = '\0';
  _cachedNwkID[0]       = '\0';
  _cachedAdrEnabled     = -1;
  _cachedDataRate       = LsmDataRate::DR_UNKNOWN;
  _cachedTxPower        = LsmTxPower::TP_UNKNOWN;
  _cachedBand           = LsmBand::BAND_UNKNOWN;
  _cachedSubBand        = -1;
  _cachedDutyCycle      = -1;
  _cachedJoin1Delay     = -1;
  _cachedJoin2Delay     = -1;
  _cachedRx1Delay       = -1;
  _cachedRx2Delay       = -1;
  _cachedRx2DataRate    = LsmDataRate::DR_UNKNOWN;
  _cachedRx2Frequency   = -1;
  _cachedConfirmRetry   = -1;
  _cachedUnconfirmRetry = -1;
}
