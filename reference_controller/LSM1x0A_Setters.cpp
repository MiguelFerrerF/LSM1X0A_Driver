#include "LSM1x0A_Controller.h"

// ---------------------------------------------------------
// SETTERS
// ---------------------------------------------------------
bool LSM1x0A_Controller::setMode(bool mode)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::MODE, mode ? 1 : 0);
  AtError err = _parser.sendCommand(cmd);
  if (err == AtError::BOOT_ALERT) {
    log(LsmLogLevel::INFO, "Modo de operación cambiado a %s correctamente.", mode ? "LoRa" : "Sigfox");
    _shadowConfig.isLoRaMode = mode;
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al cambiar modo de operación a %s.", mode ? "LoRa" : "Sigfox");
  return false;
}

bool LSM1x0A_Controller::setAppEUI(const char* appEui)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_EUI, appEui);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "AppEUI configurada: %s", appEui);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar AppEUI: %s", appEui);
  return false;
}

bool LSM1x0A_Controller::setDevEUI(const char* devEui)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_EUI, devEui);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "DevEUI configurado: %s", devEui);
    strncpy(_shadowConfig.devEui, devEui, sizeof(_shadowConfig.devEui) - 1);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar DevEUI: %s", devEui);
  return false;
}

bool LSM1x0A_Controller::setKeys(const char* devEui, const char* appKey, const char* nwkKey, const char* appEui)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  // Configurar DevEUI
  if (devEui != nullptr) {
    if (!setDevEUI(devEui))
      return false;
  }

  // Configurar AppKey
  if (appKey != nullptr) {
    if (!setAppKey(appKey))
      return false;
  }

  // Configurar NwkKey
  if (nwkKey != nullptr) {
    if (!setNwkKey(nwkKey))
      return false;
  }

  // Configurar AppEUI
  if (appEui != nullptr) {
    if (!setAppEUI(appEui))
      return false;
  }
  return true;
}

bool LSM1x0A_Controller::setAppKey(const char* appKey)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_KEY, appKey);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "AppKey configurada: %s", appKey);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar AppKey: %s", appKey);
  return false;
}

bool LSM1x0A_Controller::setNwkKey(const char* nwkKey)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_KEY, nwkKey);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "NwkKey configurada: %s", nwkKey);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar NwkKey: %s", nwkKey);
  return false;
}

bool LSM1x0A_Controller::setDevAddr(const char* devAddr)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_ADDR, devAddr);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "DevAddr configurado: %s", devAddr);
    strncpy(_shadowConfig.devAddr, devAddr, sizeof(_shadowConfig.devAddr) - 1);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar DevAddr: %s", devAddr);
  return false;
}

bool LSM1x0A_Controller::setAppSKey(const char* appSKey)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_SKEY, appSKey);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "AppSKey configurada: %s", appSKey);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar AppSKey: %s", appSKey);
  return false;
}

bool LSM1x0A_Controller::setNwkSKey(const char* nwkSKey)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_SKEY, nwkSKey);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "NwkSKey configurada: %s", nwkSKey);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar NwkSKey: %s", nwkSKey);
  return false;
}

bool LSM1x0A_Controller::setNwkID(const char* nwkID)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_ID, nwkID);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "NwkID configurada: %s", nwkID);
    strncpy(_shadowConfig.nwkID, nwkID, sizeof(_shadowConfig.nwkID) - 1);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar NwkID: %s", nwkID);
  return false;
}

bool LSM1x0A_Controller::setDevNonce(uint32_t devNonce)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s%u", LsmAtCommand::DEVNONCE, devNonce);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "DevNonce configurado: %u", devNonce);
    return true;
  }
  log(LsmLogLevel::ERROR, "Fallo al configurar DevNonce: %u", devNonce);
  return false;
}

bool LSM1x0A_Controller::setADR(bool enabled)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ADAPTIVE_DR, enabled ? 1 : 0);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "ADR configurado: %s", enabled ? "Habilitado" : "Deshabilitado");
    _shadowConfig.adrEnabled = enabled;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setDataRate(LsmDataRate dr)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  if (_shadowConfig.adrEnabled) {
    log(LsmLogLevel::ERROR, "No se puede configurar Data Rate cuando ADR está habilitado.");
    return false;
  }
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::DR, dr);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Data Rate configurado: DR%d", dr);
    _shadowConfig.dr = dr;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setTxPower(LsmTxPower tp)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::TX_POWER, tp);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Transmit Power configurado: %d", tp);
    _shadowConfig.txPower = tp;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setBand(LsmBand band)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  // Check if band is AS923_4 or AS923_1
  uint8_t subBand = 0;
  if (band == LsmBand::AS923_4 || band == LsmBand::AS923_1) {
    log(LsmLogLevel::LSM_DEBUG, "Banda AS923 seleccionada: %d", LsmBand::AS923_1 == band ? 1 : 4);
    subBand = (band == LsmBand::AS923_4) ? 4 : 1;
    band    = LsmBand::AS923_1; // Usar comando AT para
  }

  char cmd[16];
  if (subBand != 0)
    snprintf(cmd, sizeof(cmd), "%s%d,%d", LsmAtCommand::BAND, band, subBand);
  else
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::BAND, band);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Banda LoRaWAN configurada: %d", band);
    _shadowConfig.rx2Frequency = getRx2Frequency(); // Actualizamos frecuencia RX2 que depende de la banda
    _shadowConfig.band         = band;
    if (subBand != 0)
      _shadowConfig.subBand = subBand;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setClass(LsmClass cls)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CLASS, cls == LsmClass::CLASS_A ? "A" : (cls == LsmClass::CLASS_B ? "B" : "C"));
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Clase LoRaWAN configurada: %c", cls == LsmClass::CLASS_A ? 'A' : (cls == LsmClass::CLASS_B ? 'B' : 'C'));
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setDutyCycle(bool enabled)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::DUTY_CYCLE, enabled ? 1 : 0);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Duty Cycle configurado: %s", enabled ? "Habilitado" : "Deshabilitado");
    _shadowConfig.dutyCycle = enabled;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setJoin1Delay(int32_t delayMilis)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN_DELAY_1, delayMilis);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Join 1 Delay configurado: %d ms", delayMilis);
    _shadowConfig.join1Delay = delayMilis;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setJoin2Delay(int32_t delayMilis)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN_DELAY_2, delayMilis);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Join 2 Delay configurado: %d ms", delayMilis);
    _shadowConfig.join2Delay = delayMilis;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRx1Delay(int32_t delayMilis)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX1_DELAY, delayMilis);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "RX1 Delay configurado: %d ms", delayMilis);
    _shadowConfig.rx1Delay = delayMilis;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRx2Delay(int32_t delayMilis)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX2_DELAY, delayMilis);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "RX2 Delay configurado: %d ms", delayMilis);
    _shadowConfig.rx2Delay = delayMilis;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRx2DataRate(LsmDataRate dr)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RX2_DR, dr);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "RX2 Data Rate configurado: DR%d", dr);
    _shadowConfig.rx2DataRate = dr;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRx2Frequency(uint32_t freqHz)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  if (freqHz < 100000000 || freqHz > 1000000000) {
    log(LsmLogLevel::ERROR, "Frecuencia RX2 fuera de rango: %lu Hz", freqHz);
    return false;
  }
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%lu", LsmAtCommand::RX2_FREQ, freqHz);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "RX2 Frequency configurada: %lu Hz", freqHz);
    _shadowConfig.rx2Frequency = freqHz;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setPingSlot(LsmPingSlot periodicity)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::PING_SLOT, static_cast<uint8_t>(periodicity));
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Ping Slot configurado: %d", static_cast<uint8_t>(periodicity));
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setLinkCheck()
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  if (executeCommandWithRetries(LsmAtCommand::LINK_CHECK) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Link Check configurado para la siguiente transmisión.");
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setConfirmRetry(uint8_t count)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  if (count > 15) {
    log(LsmLogLevel::ERROR, "Confirm Retry máximo es 15. Ajustando a 15.");
    count = 15;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::CONFIRM_RETRY, count);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Confirm Retry configurado: %d", count);
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setUnconfirmedRetry(uint8_t count)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  if (count > 15) {
    log(LsmLogLevel::ERROR, "Unconfirmed Retry máximo es 15. Ajustando a 15.");
    count = 15;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::UNCONFIRM_RETRY, count);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Unconfirmed Retry configurado: %d", count);
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setNwkType(LsmNetworkType type)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::NETWORK_TYPE, type);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Nwk Type configurado: %s", type == LsmNetworkType::PUBLIC ? "PUBLIC" : "PRIVATE");
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setChannelMaskBySubBand(LsmBand band, int subBand)
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  const LsmMaskConfig* target = nullptr;
  for (const auto& item : BAND_MAPS) {
    if (item.band == band) {
      target = &item;
      break;
    }
  }

  if (!target) {
    log(LsmLogLevel::ERROR, "Banda %d no soportada para Channel Mask", (int)band);
    return false;
  }

  int idx = (subBand >= 0 && subBand < target->count) ? subBand : target->count;

  char cmd[80];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CHANNEL_MASK, target->masks[idx]);
  log(LsmLogLevel::INFO, "Configurando máscara: %s", target->masks[idx]);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Channel Mask configurada para Banda %d Sub-Banda %d", (int)band, idx);
    _shadowConfig.band               = band;
    _shadowConfig.channelMaskSubBand = subBand;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRCChannel(LsmRCChannel channel)
{
  if (_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  if (channel == LsmRCChannel::RC3A || channel == LsmRCChannel::RC3C) {
    char ch[3];
    snprintf(ch, sizeof(ch), "%s", (channel == LsmRCChannel::RC3A) ? "3A" : "3C");
    snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RC_CHANNEL, ch);
  }
  else
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RC_CHANNEL, static_cast<uint8_t>(channel));
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "RC Channel configurado: %d", static_cast<uint8_t>(channel));
    _shadowConfig.rcChannel = channel;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setRadioPower(uint8_t power)
{
  if (_shadowConfig.isLoRaMode)
    return false;

  if (power > 20) {
    log(LsmLogLevel::ERROR, "Radio Power máximo es 20 dBm. Ajustando a 20 dBm.");
    power = 20;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::RADIO_POWER, power);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Radio Power configurado: %d dBm", power);
    _shadowConfig.radioPower = power;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setEncryptKey(bool privateKey)
{
  if (_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ENCRYPT_KEY, privateKey ? 0 : 1);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Encrypt Key configurada: %s", privateKey ? "Privada" : "Pública");
    _shadowConfig.encryptKey = privateKey;
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::setEncryptPayload(bool enabled)
{
  if (_shadowConfig.isLoRaMode)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::ENCRYPT_PAYLOAD, enabled ? 1 : 0);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    log(LsmLogLevel::LSM_DEBUG, "Encrypt Payload configurado: %s", enabled ? "Habilitado" : "Deshabilitado");
    _shadowConfig.encryptPayload = enabled;
    return true;
  }
  return false;
}
