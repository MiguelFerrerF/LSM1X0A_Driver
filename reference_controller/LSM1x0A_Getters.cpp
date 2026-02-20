#include "LSM1x0A_Controller.h"

// ---------------------------------------------------------
// GETTERS
// ---------------------------------------------------------
bool LSM1x0A_Controller::isLSM110A()
{
  return _isLSM110A;
}

uint32_t LSM1x0A_Controller::getBaudrate()
{
  char buffer[16] = {0};
  char cmd[32];
  // BAUDRATE constant comes with '=' in typical cases, but sometimes getters have ?. 
  // For safety simply send 'AT+BAUDRATE=?' unless the constant lacks the =.
  // Actually, standard is AT+BAUDRATE=? according to manual, or we append ? to AT+BAUDRATE=
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::BAUDRATE, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) == AtError::OK) {
    return strtoul(buffer, nullptr, 10);
  }
  return LSM1X0A_BAUDRATE;
}

uint8_t LSM1x0A_Controller::getVerboseLevel()
{
  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::VERBOSE_LEVEL, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) == AtError::OK) {
    return atoi(buffer);
  }
  return 0;
}

uint8_t LSM1x0A_Controller::getBattery()
{
  char buffer[16] = {0};
  if (executeCommandWithRetries(LsmAtCommand::BATTERY, nullptr, buffer, sizeof(buffer)) == AtError::OK) {
    return atoi(buffer);
  }
  return 0;
}

void LSM1x0A_Controller::getLocalTime(char* outBuffer, size_t outSize)
{
  if (!outBuffer || outSize == 0) return;
  outBuffer[0] = '\0';
  char tempBuffer[64] = {0};
  if (executeCommandWithRetries(LsmAtCommand::LOCAL_TIME, nullptr, tempBuffer, sizeof(tempBuffer)) == AtError::OK) {
      // Return is usually "LTIME:01h59m44s on 01/01/1970", we can keep it as is, or strip "LTIME:"
      const char* prefix = "LTIME:";
      if (strncmp(tempBuffer, prefix, strlen(prefix)) == 0) {
          strncpy(outBuffer, tempBuffer + strlen(prefix), outSize - 1);
      } else {
          strncpy(outBuffer, tempBuffer, outSize - 1);
      }
      outBuffer[outSize - 1] = '\0';
  }
}

bool LSM1x0A_Controller::getMode()
{
  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::MODE, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    log(LsmLogLevel::ERROR, "Error al obtener modo de operación");
    return false;
  }
  return (strcmp(buffer, "1") == 0);
}

void LSM1x0A_Controller::getVersion(char* outBuffer, size_t outSize)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s", LsmAtCommand::FW_VERSION);
  if (executeCommandWithRetries(cmd, "APP_VERSION:", outBuffer, outSize) == AtError::OK) {
    // Parser returns exactly from after the matched expected tag. 
    // It might have spaces. Strip spaces.
    if (outSize > 0 && outBuffer[0] != '\0') {
        char* val = outBuffer;
        while (*val == ' ' || *val == '\t') val++;
        if (val != outBuffer) {
            memmove(outBuffer, val, strlen(val) + 1);
        }
    }
  } else {
    if (outSize > 0)
      outBuffer[0] = '\0';
  }
}

void LSM1x0A_Controller::getAppEUI(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_EUI, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getDevEUI(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_EUI, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getKeys(char* devEui, size_t devEuiSize, char* appKey, size_t appKeySize, char* nwkKey, size_t nwkKeySize, char* appEui,
                                 size_t appEuiSize)
{
  getDevEUI(devEui, devEuiSize);
  getAppKey(appKey, appKeySize);
  getNwkKey(nwkKey, nwkKeySize);
  getAppEUI(appEui, appEuiSize);
}

void LSM1x0A_Controller::getAppKey(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_KEY, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getNwkKey(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_KEY, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getDevAddr(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_ADDR, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getAppSKey(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_SKEY, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getNwkSKey(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_SKEY, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getNwkID(char* outBuffer, size_t outSize)
{
  if (!_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_ID, "?");
  if (executeCommandWithRetries(cmd, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

uint32_t LSM1x0A_Controller::getDevNonce()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEVNONCE, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0;
  }
  return static_cast<uint32_t>(atoi(buffer));
}

bool LSM1x0A_Controller::getADR()
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::ADAPTIVE_DR, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return false; // Valor por defecto en caso de error
  }
  return (strcmp(buffer, "1") == 0);
}

LsmDataRate LSM1x0A_Controller::getDataRate()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmDataRate::DR_0;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DR, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmDataRate::DR_0; // Valor por defecto en caso de error
  }
  return static_cast<LsmDataRate>(atoi(buffer));
}

LsmTxPower LSM1x0A_Controller::getTxPower()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmTxPower::TP_MAX;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::TX_POWER, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmTxPower::TP_MAX; // Valor por defecto en caso de error
  }
  return static_cast<LsmTxPower>(atoi(buffer));
}

LsmBand LSM1x0A_Controller::getBand()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmBand::EU868;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::BAND, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmBand::EU868; // Valor por defecto en caso de error
  }
  return static_cast<LsmBand>(atoi(buffer));
}

LsmClass LSM1x0A_Controller::getClass()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmClass::CLASS_A;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CLASS, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmClass::CLASS_C; // Valor por defecto en caso de error
  }
  return static_cast<LsmClass>(atoi(buffer));
}

bool LSM1x0A_Controller::getDutyCycle()
{
  if (!_shadowConfig.isLoRaMode)
    return false;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DUTY_CYCLE, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return false; // Valor por defecto en caso de error
  }
  return (strcmp(buffer, "ON") == 0);
}

int32_t LSM1x0A_Controller::getJoin1Delay()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::JOIN_DELAY_1, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return atoi(buffer);
}

int32_t LSM1x0A_Controller::getJoin2Delay()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::JOIN_DELAY_2, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return atoi(buffer);
}

int32_t LSM1x0A_Controller::getRx1Delay()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX1_DELAY, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return atoi(buffer);
}

int32_t LSM1x0A_Controller::getRx2Delay()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_DELAY, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return atoi(buffer);
}

LsmDataRate LSM1x0A_Controller::getRx2DataRate()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmDataRate::DR_0;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_DR, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmDataRate::DR_0; // Valor por defecto en caso de error
  }
  return static_cast<LsmDataRate>(atoi(buffer));
}

uint32_t LSM1x0A_Controller::getRx2Frequency()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_FREQ, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return strtoul(buffer, nullptr, 10);
}

LsmPingSlot LSM1x0A_Controller::getPingSlot()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmPingSlot::EVERY_1_SEC;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::PING_SLOT, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmPingSlot::EVERY_1_SEC; // Valor por defecto en caso de error
  }
  return static_cast<LsmPingSlot>(atoi(buffer));
}

uint8_t LSM1x0A_Controller::getConfirmRetry()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CONFIRM_RETRY, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return static_cast<uint8_t>(atoi(buffer));
}

uint8_t LSM1x0A_Controller::getUnconfirmedRetry()
{
  if (!_shadowConfig.isLoRaMode)
    return 0;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::UNCONFIRM_RETRY, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return static_cast<uint8_t>(atoi(buffer));
}

LsmNetworkType LSM1x0A_Controller::getNwkType()
{
  if (!_shadowConfig.isLoRaMode)
    return LsmNetworkType::PUBLIC;

  char buffer[16] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NETWORK_TYPE, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmNetworkType::PUBLIC; // Valor por defecto en caso de error
  }
  return (strcmp(buffer, "Public Mode") == 0) ? LsmNetworkType::PUBLIC : LsmNetworkType::PRIVATE;
}

void LSM1x0A_Controller::getChannelMask(char* outBuffer, size_t size)
{
  if (!_shadowConfig.isLoRaMode) {
    if (size > 0)
      outBuffer[0] = '\0';
    return;
  }

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CHANNEL_MASK, "?");
  if (executeCommandWithRetries(cmd, "channel_mask", outBuffer, size) != AtError::OK) {
    if (size > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getDevID(char* outBuffer, size_t outSize)
{
  if (_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  if (executeCommandWithRetries(LsmAtCommand::DEV_ID, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

void LSM1x0A_Controller::getDevPAC(char* outBuffer, size_t outSize)
{
  if (_shadowConfig.isLoRaMode) {
    if (outSize > 0)
      outBuffer[0] = '\0';
    return;
  }

  if (executeCommandWithRetries(LsmAtCommand::DEV_PAC, nullptr, outBuffer, outSize) != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

LsmRCChannel LSM1x0A_Controller::getRCChannel()
{
  if (_shadowConfig.isLoRaMode)
    return LsmRCChannel::RC1;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RC_CHANNEL, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return LsmRCChannel::RC1; // Valor por defecto en caso de error
  }
  if (strcmp(buffer, "3A") == 0)
    return LsmRCChannel::RC3A;
  if (strcmp(buffer, "3C") == 0)
    return LsmRCChannel::RC3C;
  return static_cast<LsmRCChannel>(atoi(buffer));
}

uint8_t LSM1x0A_Controller::getRadioPower()
{
  if (_shadowConfig.isLoRaMode)
    return 0;

  char buffer[8] = {0};
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RADIO_POWER, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return 0; // Valor por defecto en caso de error
  }
  return static_cast<uint8_t>(atoi(buffer));
}

bool LSM1x0A_Controller::getEncryptKey()
{
  if (_shadowConfig.isLoRaMode)
    return false;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::ENCRYPT_KEY, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return false; // Valor por defecto en caso de error
  }
  return (strcmp(buffer, "1") == 0);
}

bool LSM1x0A_Controller::getEncryptPayload()
{
  if (_shadowConfig.isLoRaMode)
    return false;

  char buffer[8] = {0};
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::ENCRYPT_PAYLOAD, "?");
  if (executeCommandWithRetries(cmd, nullptr, buffer, sizeof(buffer)) != AtError::OK) {
    return false; // Valor por defecto en caso de error
  }
  return (strcmp(buffer, "1") == 0);
}

int16_t LSM1x0A_Controller::getLastRxRSSI()
{
  return _currentRSSI;
}
