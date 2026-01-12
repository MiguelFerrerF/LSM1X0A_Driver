#include "LoRaWANController.h"
#include <cstdio>
#include <cstring>

LoRaWANController *LoRaWANController::_instance = nullptr;

LoRaWANController::LoRaWANController()
    : _joined(false), _hwResetCb(nullptr), _serialPort(HardwareSerial(2)),
      _driver(_serialPort) {
  _instance = this;
}

bool LoRaWANController::begin(unsigned long baudRate, int rxPin, int txPin) {
  if (!_parser.init()) {
    return false;
  }

  _parser.registerEventCallback(_eventHandlerWrapper);

  // Ahora pasamos la función con la firma correcta (const uint8_t*, size_t)
  bool driverStarted = _driver.init(baudRate, rxPin, txPin, _serialRxCallback);

  return driverStarted;
}

// ---------------------------------------------------------
// PUENTES ESTÁTICOS (Callbacks)
// ---------------------------------------------------------

// CORRECCIÓN: Recibimos un bloque de datos del driver
void LoRaWANController::_serialRxCallback(const uint8_t *data, size_t len) {
  if (_instance && data != nullptr && len > 0) {
    // Iteramos sobre el buffer recibido e inyectamos byte a byte al parser
    for (size_t i = 0; i < len; i++) {
      // Casting seguro a char para el parser
      _instance->_parser.processByte((char)data[i]);
    }
  }
}

void LoRaWANController::_eventHandlerWrapper(const char *evt) {
  if (_instance) {
    _instance->_processEvent(evt);
  }
}

// ---------------------------------------------------------
// LÓGICA PRIVADA
// ---------------------------------------------------------

void LoRaWANController::_processEvent(const char *evt) {
  if (strstr(evt, "JOINED") != nullptr) {
    _joined = true;
  } else if (strstr(evt, "JOIN FAILED") != nullptr) {
    _joined = false;
  }
}

// ---------------------------------------------------------
// LÓGICA DE RESET INTELIGENTE
// ---------------------------------------------------------
bool LoRaWANController::smartReset(char *outVersion, uint16_t verLen) {
  _joined = false;
  ATResult_t res = AT_RES_ERROR;

  const char *resetCmd = "ATZ\r\n";
  _driver.write(resetCmd, strlen(resetCmd));

  res = _waitForBoot(outVersion, verLen, 2000);

  if (res == AT_RES_OK) {
    return true;
  }

  if (_hwResetCb != nullptr) {
    _hwResetCb(true); // LOW
    vTaskDelay(pdMS_TO_TICKS(100));
    _hwResetCb(false); // HIGH

    res = _waitForBoot(outVersion, verLen, 3000);

    return (res == AT_RES_OK);
  }

  return false;
}

ATResult_t LoRaWANController::_waitForBoot(char *outVer, uint16_t len,
                                           uint32_t timeoutMs) {
  _parser.setExpectation(AT_TYPE_BOOT, outVer, len);

  if (xSemaphoreTake(_parser.getSemaphore(), pdMS_TO_TICKS(timeoutMs)) ==
      pdTRUE) {
    return _parser.getStatus();
  }

  _parser.abort();
  return AT_RES_TIMEOUT;
}

// ---------------------------------------------------------
// HELPERS GENÉRICOS DE TRANSMISIÓN
// ---------------------------------------------------------

ATResult_t LoRaWANController::_sendExec(const char *cmd, uint32_t timeoutMs) {
  _parser.setExpectation(AT_TYPE_EXEC);

  _driver.write(cmd, strlen(cmd));
  _driver.write("\r\n", 2);

  if (xSemaphoreTake(_parser.getSemaphore(), pdMS_TO_TICKS(timeoutMs)) ==
      pdTRUE) {
    return _parser.getStatus();
  }
  _parser.abort();
  return AT_RES_TIMEOUT;
}

ATResult_t LoRaWANController::_sendQuery(const char *cmd, char *outBuffer,
                                         uint16_t len, uint32_t timeoutMs) {
  _parser.setExpectation(AT_TYPE_QUERY, outBuffer, len);

  _driver.write(cmd, strlen(cmd));
  _driver.write("\r\n", 2);

  if (xSemaphoreTake(_parser.getSemaphore(), pdMS_TO_TICKS(timeoutMs)) ==
      pdTRUE) {
    return _parser.getStatus();
  }
  _parser.abort();
  return AT_RES_TIMEOUT;
}

// ---------------------------------------------------------
// COMANDOS DE APLICACIÓN
// ---------------------------------------------------------

bool LoRaWANController::setClass(char classType) {
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+CLASS=%c", classType);
  return (_sendExec(cmd) == AT_RES_OK);
}

bool LoRaWANController::getVersion(char *buffer, uint16_t maxLen) {
  return (_sendQuery("AT+VER?", buffer, maxLen) == AT_RES_OK);
}

bool LoRaWANController::join(uint8_t mode) {
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+JOIN=%d", mode);

  ATResult_t res = _sendExec(cmd, 3000);

  if (res == AT_RES_OK) {
    _joined = false;
    return true;
  }
  return false;
}

bool LoRaWANController::sendData(uint8_t port, const char *hexData,
                                 bool confirmed) {
  char cmd[256];
  uint8_t ack = confirmed ? 1 : 0;

  int n = snprintf(cmd, sizeof(cmd), "AT+SEND=%d:%d:%s", port, ack, hexData);
  if (n >= sizeof(cmd))
    return false;

  return (_sendExec(cmd, 5000) == AT_RES_OK);
}

bool LoRaWANController::isJoined() const { return _joined; }