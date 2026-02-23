#ifndef LSM1X0A_AT_PARSER_H
#define LSM1X0A_AT_PARSER_H

#include "LSM1x0A_Types.h"
#include "UartDriver.h"
#include "freertos/semphr.h"
#include <cstdio>
#include <cstring>

// Callback para eventos asíncronos (URC)
// type: "RX", "JOIN", etc.
// payload: El resto de la linea
// ctx: Contexto de usuario
typedef void (*AtEventCallback)(const char* type, const char* payload, void* ctx);

class LSM1x0A_AtParser
{
public:
  LSM1x0A_AtParser();
  ~LSM1x0A_AtParser();

  // Inicialización
  bool init(UartDriver* driver, AtEventCallback onEvent = nullptr, void* eventCtx = nullptr);

  /**
   * @brief Realiza un "Ping" de activación enviando comandos AT hasta recibir OK.
   * Maneja el estado de bajo consumo del módulo.
   * @return true si el módulo despertó y respondió adecuadamente.
   */
  bool wakeUp(uint8_t retries = DEFAULT_MAX_RETRIES, uint32_t delayMs = 500);

  /**
   * @brief Envía comando simple y espera OK/ERROR.
   * Ejemplo: sendCommand("AT+JOIN");
   */
  AtError sendCommand(const char* cmd, uint32_t timeoutMs = 2000);

  /**
   * @brief Espera a que ocurra un evento (como BOOTALERT) sin enviar ningún comando.
   * Útil para reset por hardware donde el módulo habla solo.
   * @return El AtError correspondiente al evento (ej. AtError::BOOT_ALERT).
   */
  AtError waitForEvent(uint32_t timeoutMs = LSM1X0A_BOOT_ALERT_TIMEOUT_MS);

  /**
   * @brief Envía comando y extrae un valor de la respuesta.
   * * @param cmd Comando a enviar (ej "AT+DEUI")
   * @param expectedTag Etiqueta que precede al valor (ej "DevEui: ") o NULL si no hay tag.
   * @param outBuffer Buffer donde copiar el resultado (NO incluye el tag).
   * @param outSize Tamaño del buffer de salida.
   * @return AtError
   */
  AtError sendCommandWithResponse(const char* cmd, const char* expectedTag, char* outBuffer, size_t outSize, uint32_t timeoutMs = 2000);

  // Método público para inyección de datos desde el Driver UART
  void eatBuffer(const uint8_t* data, size_t len);

  // Metodo público para descripción de errores
  const char* atErrorToString(AtError err);

  /**
   * @brief Helper estático para convertir el string de metadatos en una struct usable.
   * Parsear: "+EVT:RX_1, PORT 2, DR 5, RSSI -90, SNR 10, DMODM 10, GWN 2"
   */
  static bool parseRxMetadata(const char* payload, LsmRxMetadata* outMeta);

  /**
   * @brief Obtiene el modelo de dispositivo detectado durante el arranque.
   */
  LsmModuleType getDeviceType() const
  {
    return _deviceType;
  }

private:
  UartDriver*     _driver        = nullptr;
  AtEventCallback _eventCallback = nullptr;
  void*           _eventCtx      = nullptr;

  LsmModuleType _deviceType = LsmModuleType::UNKNOWN;

  // Buffer Interno Estático (Cero asignación dinámica)
  char     _lineBuffer[AT_BUFFER_SIZE] = {0};
  uint16_t _lineIdx                    = 0;

  // Sincronización
  SemaphoreHandle_t _syncSem = nullptr;

  // Estado de la Transacción Actual
  bool    _pendingCommand  = false;
  AtError _lastResultError = AtError::UNKNOWN;

  // Punteros para parsing "Zero-Copy" durante la transacción
  const char* _expectedTag   = nullptr; // El tag que esperamos en la respuesta (ej. "DevEui: ")
  char*       _userOutBuffer = nullptr; // Donde el usuario quiere que copiemos el resultado (ej. un buffer local o global)
  size_t      _userOutSize   = 0;       // Tamaño del buffer de salida para evitar sobreescrituras

  // Métodos internos
  void    processLine(char* line);
  AtError parseErrorString(const char* line);

  // Puente estático
  static void staticRxCallback(void* ctx, uint8_t* data, size_t len);
};

#endif // LSM1X0A_AT_PARSER_H