#include "../LSM1x0A_Controller.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Local helper to parse a hex string with optional separators (like ':') into a byte array.
static size_t parseHexStringToArray(const char* hexString, uint8_t* outArray, size_t arraySize)
{
  if (!hexString || !outArray || arraySize == 0)
    return 0;

  size_t      count = 0;
  const char* ptr   = hexString;
  while (*ptr && count < arraySize) {
    if (*ptr == ':' || *ptr == '-' || *ptr == ' ') {
      ptr++;
      continue;
    }
    if (isxdigit((unsigned char)ptr[0]) && isxdigit((unsigned char)ptr[1])) {
      char byteStr[3]   = {ptr[0], ptr[1], '\0'};
      outArray[count++] = (uint8_t)strtol(byteStr, NULL, 16);
      ptr += 2;
    }
    else
      break;
  }
  return count;
}

// Local helper to extract a string after a certain prefix in a buffer, or if no prefix just copy it trimming newlines.
static void extractString(const char* response, char* outBuffer, size_t size)
{
  if (!response || !outBuffer || size == 0)
    return;

  // The module's serial port sometimes responds with "DevEUI: 01020304..." but in LsmAtParser we already clean it
  // Generally for getters, it's the raw string plus \r\n. For example "0102030405060708"
  strncpy(outBuffer, response, size - 1);
  outBuffer[size - 1] = '\0';

  // Trim any trailing newline characters
  char* newline = strpbrk(outBuffer, "\r\n");
  if (newline)
    *newline = '\0';
}

bool LSM1x0A_LoRaWAN::getLocalTime(struct tm* timeinfo)
{
  if (!timeinfo)
    return false;

  char buffer[64];
  if (_controller->sendCommandWithResponse(LsmAtCommand::LOCAL_TIME, buffer, sizeof(buffer), "LTIME:", 1000) != AtError::OK)
    return false;

  // The parser removes the prefixes, so buffer should contain something like "12h34m56s on 23/02/2026"
  int h = 0, m = 0, s = 0, D = 0, M = 0, Y = 0;
  if (sscanf(buffer, "%dh%dm%ds on %d/%d/%d", &h, &m, &s, &D, &M, &Y) == 6) {
    timeinfo->tm_hour = h;
    timeinfo->tm_min  = m;
    timeinfo->tm_sec  = s;
    timeinfo->tm_mday = D;
    timeinfo->tm_mon  = M - 1;    // struct tm uses 0-11 for months
    timeinfo->tm_year = Y - 1900; // struct tm uses years since 1900
    return true;
  }

  return false;
}

int LSM1x0A_LoRaWAN::getBaudrate()
{

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::BAUDRATE);
  char buf[32];
  if (_controller->sendCommandWithResponse(cmd, buf, sizeof(buf), "Set BaudRate: ", 1000) != AtError::OK) {
    return -1;
  }

  return atoi(buf);
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

bool LSM1x0A_LoRaWAN::getDevEUI(uint8_t* outArray, size_t arraySize)
{
  char rx[64] = {0};
  if (getDevEUI(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getAppEUI(uint8_t* outArray, size_t arraySize)
{
  char rx[64] = {0};
  if (getAppEUI(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getAppKey(uint8_t* outArray, size_t arraySize)
{
  char rx[128] = {0};
  if (getAppKey(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getNwkKey(uint8_t* outArray, size_t arraySize)
{
  char rx[128] = {0};
  if (getNwkKey(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getDevAddr(uint8_t* outArray, size_t arraySize)
{
  char rx[64] = {0};
  if (getDevAddr(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getAppSKey(uint8_t* outArray, size_t arraySize)
{
  char rx[128] = {0};
  if (getAppSKey(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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

bool LSM1x0A_LoRaWAN::getNwkSKey(uint8_t* outArray, size_t arraySize)
{
  char rx[128] = {0};
  if (getNwkSKey(rx, sizeof(rx))) {
    return parseHexStringToArray(rx, outArray, arraySize) > 0;
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
  // Format can be A, B, or C
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

bool LSM1x0A_LoRaWAN::getChannelMask(uint16_t* outMasks, size_t* outArraySize)
{
  if (!outMasks || !outArraySize || *outArraySize == 0)
    return false;

  // Clear any previous data in the controller's temporary mask buffer before sending the command
  _controller->resetTempMaskBuffer();

  char cmd[16] = {0};
  snprintf(cmd, sizeof(cmd), "%s%s", LsmAtCommand::CHANNEL_MASK, "?");

  // Execute AT+CHMASK? with sendCommand, this ensures the parser waits for 'OK'
  // While waiting, the parser will iterate through the lines and fill evt->tempMaskBuffer
  if (_controller->sendCommand(cmd, 2000) == AtError::OK) {
    int count = _controller->getTempMaskCount();
    if (count > 0 && count <= (int)(*outArraySize)) {
      memcpy(outMasks, _controller->getTempMaskBuffer(), count * sizeof(uint16_t));
      *outArraySize = count;
      return true;
    }
  }

  *outArraySize = 0;
  return false;
}

int LSM1x0A_LoRaWAN::getAbpFrameCounter()
{
  if (!_controller)
    return -1;
  char valStr[16] = {0};
  char cmd[32]    = {0};
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::FRAME_CNT);
  if (_controller->sendCommandWithResponse(cmd, valStr, sizeof(valStr)) == AtError::OK) {
    return atoi(valStr);
  }
  return -1;
}
