#include "../LSM1x0A_Controller.h"
#include "LSM1x0A_LoRaWAN.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper local para extraer una cadena tras cierto prefijo en un buffer,
// o si no hay prefijo sólo copiarla quitando salto de línea.
static void extractString(const char* response, char* outBuffer, size_t size)
{
  if (!response || !outBuffer || size == 0)
    return;

  // El puerto serie del módulo a veces contesta "DevEUI: 01020304..." pero en LsmAtParser ya limpiamos
  // Generalmente para getters es la cadena cruda más \r\n. Por ejemplo "0102030405060708"
  strncpy(outBuffer, response, size - 1);
  outBuffer[size - 1] = '\0';

  // Eliminar \r o \n finales si los hubiera
  char* newline = strpbrk(outBuffer, "\r\n");
  if (newline)
    *newline = '\0';
}

bool LSM1x0A_LoRaWAN::getDevEUI(char* outBuffer, size_t size)
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_EUI, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getAppEUI(char* outBuffer, size_t size)
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_EUI, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getAppKey(char* outBuffer, size_t size)
{
  char rx[128] = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_KEY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getNwkKey(char* outBuffer, size_t size)
{
  char rx[128] = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_KEY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getDevAddr(char* outBuffer, size_t size)
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEV_ADDR, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getAppSKey(char* outBuffer, size_t size)
{
  char rx[128] = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::APP_SKEY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

bool LSM1x0A_LoRaWAN::getNwkSKey(char* outBuffer, size_t size)
{
  char rx[128] = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_SKEY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    extractString(rx, outBuffer, size);
    return true;
  }
  return false;
}

int LSM1x0A_LoRaWAN::getNwkID()
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NWK_ID, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getDevNonce()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DEVNONCE, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getADR()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::ADAPTIVE_DR, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

LsmDataRate LSM1x0A_LoRaWAN::getDataRate()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DR, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return (LsmDataRate)atoi(rx);
  }
  return LsmDataRate::DR_UNKNOWN;
}

LsmTxPower LSM1x0A_LoRaWAN::getTxPower()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::TX_POWER, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return (LsmTxPower)atoi(rx);
  }
  return LsmTxPower::TP_UNKNOWN;
}

LsmBand LSM1x0A_LoRaWAN::getBand()
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::BAND, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    int band = atoi(rx);
    return (LsmBand)band;
  }
  return LsmBand::BAND_UNKNOWN;
}

LsmClass LSM1x0A_LoRaWAN::getClass()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CLASS, "?");
  // Formato puede ser A, B o C
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    if (strchr(rx, 'B'))
      return LsmClass::CLASS_B;
    if (strchr(rx, 'C'))
      return LsmClass::CLASS_C;
    return LsmClass::CLASS_A;
  }
  return LsmClass::CLASS_A; // Default safe via timeout
}

int LSM1x0A_LoRaWAN::getDutyCycle()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::DUTY_CYCLE, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getJoin1Delay()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::JOIN_DELAY_1, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getJoin2Delay()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::JOIN_DELAY_2, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getRx1Delay()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX1_DELAY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getRx2Delay()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_DELAY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atoi(rx);
  }
  return -1;
}

LsmDataRate LSM1x0A_LoRaWAN::getRx2DataRate()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_DR, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return (LsmDataRate)atoi(rx);
  }
  return LsmDataRate::DR_UNKNOWN;
}

long LSM1x0A_LoRaWAN::getRx2Frequency()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::RX2_FREQ, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return atol(rx);
  }
  return -1;
}

LsmPingSlot LSM1x0A_LoRaWAN::getPingSlot()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::PING_SLOT, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    return (LsmPingSlot)atoi(rx);
  }
  return LsmPingSlot::PING_SLOT_UNKNOWN;
}

int LSM1x0A_LoRaWAN::getConfirmRetry()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CONFIRM_RETRY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    _cachedConfirmRetry = atoi(rx);
    return _cachedConfirmRetry;
  }
  return -1;
}

int LSM1x0A_LoRaWAN::getUnconfirmRetry()
{
  char rx[32]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::UNCONFIRM_RETRY, "?");
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    _cachedUnconfirmRetry = atoi(rx);
    return _cachedUnconfirmRetry;
  }
  return -1;
}

LsmNetworkType LSM1x0A_LoRaWAN::getNetworkType()
{
  char rx[64]  = {0};
  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::NETWORK_TYPE, "?");
  // Modulo envuelve "Public Mode" o "Private Mode" en la respuesta
  if (_controller->sendCommandWithResponse(cmd, rx, sizeof(rx), nullptr, 1000) == AtError::OK) {
    if (strstr(rx, "Public"))
      return LsmNetworkType::PUBLIC; // Public
    if (strstr(rx, "Private"))
      return LsmNetworkType::PRIVATE; // Private
    return LsmNetworkType::NETWORK_TYPE_UNKNOWN;
  }
  return LsmNetworkType::NETWORK_TYPE_UNKNOWN;
}
