#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "stdarg.h" // Para manejo de argumentos variables

typedef void (*LsmUserCallback)(const char* type, const char* payload);

class LSM1x0A_Controller
{
public:
  LSM1x0A_Controller();
  ~LSM1x0A_Controller() = default;

  /**
   * @brief Inicializa el controlador.
   * @param driver Puntero al driver UART ya configurado.
   * @param resetPin Pin GPIO conectado al RST del módulo (-1 si no se usa).
   * @param callback Función para recibir eventos.
   */
  bool begin(UartDriver* driver, int resetPin = -1, LsmUserCallback callback = nullptr);

  // Método público por si el usuario quiere forzar un reset manual en runtime
  bool hardReset();
  bool softReset();

  /**
   * @brief Define qué tanta información queremos recibir en el callback.
   * @param level Nivel deseado (ej. LsmLogLevel::INFO)
   */
  void setLogLevel(LsmLogLevel level);

  // Métodos de Negocio
  bool join(bool isOTAA = true);
  bool sendData(uint8_t port, const char* data, bool confirmed = false);
  bool setKeys(const char* devEui = nullptr, const char* appKey = nullptr, const char* appEui = nullptr);
  bool setBand(uint8_t band);
  void getDevEUI(char* outBuffer, size_t outSize);

  // Estado del Módulo
  bool isJoined() const;
  bool isTxBusy() const;

private:
  LSM1x0A_AtParser _parser;
  LsmUserCallback  _userCallback = nullptr;

  // Reset pin
  int _resetPin;

  // Estado interno
  bool _isJoined;
  bool _isTxBusy;

  // Configuración de Logging
  LsmLogLevel _currentLogLevel = LsmLogLevel::INFO;

  // Helper interno para generar logs formateados
  void log(LsmLogLevel level, const char* format, ...);

  static void parserEventProxy(const char* type, const char* payload, void* ctx);
  void        handleInternalEvent(const char* type, const char* payload);
};
#endif // LSM1X0A_CONTROLLER_H