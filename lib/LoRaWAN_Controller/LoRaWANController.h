#ifndef LORAWAN_CONTROLLER_H
#define LORAWAN_CONTROLLER_H

#include "ATParser.h"
#include "SerialDriver.h"
#include <stddef.h> // Para size_t
#include <stdint.h>

class LoRaWANController {
public:
  LoRaWANController();

  // Inicialización
  bool begin(unsigned long baudRate, int rxPin, int txPin);

  // Callbacks de Hardware
  void setHardwareResetCallback(HwResetCallback cb) { _hwResetCb = cb; }

  // --- Comandos de Aplicación ---
  bool smartReset(char *outVersion = nullptr, uint16_t verLen = 0);
  bool setClass(char classType);
  bool getVersion(char *buffer, uint16_t maxLen);
  bool join(uint8_t mode = 1);
  bool isJoined() const;
  bool sendData(uint8_t port, const char *hexData, bool confirmed);

private:
  HardwareSerial _serialPort;
  SerialDriver _driver;
  ATParser _parser;
  bool _joined;
  HwResetCallback _hwResetCb;

  static LoRaWANController *_instance;

  // --- Callbacks Estáticos ---

  // CORRECCIÓN: Ahora coincide con OnReceiveCallback (buffer + longitud)
  static void _serialRxCallback(const uint8_t *data, size_t len);

  static void _eventHandlerWrapper(const char *evt);

  // --- Métodos Internos ---
  void _processEvent(const char *evt);

  ATResult_t _sendExec(const char *cmd,
                       uint32_t timeoutMs = AT_DEFAULT_TIMEOUT);
  ATResult_t _sendQuery(const char *cmd, char *outBuffer, uint16_t len,
                        uint32_t timeoutMs = AT_DEFAULT_TIMEOUT);
  ATResult_t _waitForBoot(char *outVer, uint16_t len, uint32_t timeoutMs);
};

#endif // LORAWAN_CONTROLLER_H