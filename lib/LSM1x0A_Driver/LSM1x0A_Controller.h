#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include "api/LSM1x0A_LoRaWAN.h"
#include <Arduino.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// Definición de bits para el EventGroup interno de sincronización
#define LSM_EVT_JOIN_SUCCESS (1 << 0)
#define LSM_EVT_JOIN_FAIL    (1 << 1)
#define LSM_EVT_TX_SUCCESS   (1 << 2)
#define LSM_EVT_TX_FAIL      (1 << 3)
#define LSM_EVT_RX_DATA      (1 << 4)
#define LSM_EVT_RX_TIMEOUT   (1 << 5)

/**
 * @class LSM1x0A_Controller
 * @brief Controlador intuitivo para interactuar con módulos LSM1x0A (LoRaWAN/Sigfox).
 *
 * Esta clase proporciona una interfaz sencilla para el usuario, abstrayendo
 * el driver UART y el parseador AT subyacentes. Permite inicializar la
 * comunicación, gestionar callbacks de eventos y enviar comandos generales
 * obteniendo sus respuestas fácilmente.
 */
class LSM1x0A_Controller
{
public:
  // =========================================================================
  // CONSTRUCTORES Y DESTRUCTORES
  // =========================================================================

  /**
   * @brief Constructor por defecto.
   * Crea las instancias internas de UartDriver y LSM1x0A_AtParser,
   * listas para ser inicializadas mediante begin().
   */
  LSM1x0A_Controller();

  /**
   * @brief Destructor.
   * Libera recursos y de-inicializa los componentes internos.
   */
  ~LSM1x0A_Controller();

  // =========================================================================
  // INICIALIZACIÓN Y CONTROL DE COMUNICACIÓN
  // =========================================================================

  /**
   * @brief Inicializa el hardware UART y el Parser interno.
   *
   * @param callback Función de callback (ej. onLoRaEvent) que el usuario
   * defina para procesar eventos asíncronos (+EVT:...).
   * @param ctx Un puntero de contexto opcional para el callback (puede ser this o nullptr).
   * @return true si la UART se inició con éxito y la memoria fue alocada, false en otro caso.
   */
  bool begin(AtEventCallback callback, void* ctx = nullptr);

  /**
   * @brief Detiene el hardware y desvincula los event listeners.
   */
  void end();

  /**
   * @brief Despierta al módulo e intenta sincronizar su estado inicial.
   * Envía comandos AT o caracteres 'Wake-Up' básicos.
   *
   * @return true si el módulo responde "OK" a la orden básica.
   */
  bool wakeUp();

  /**
   * @brief Envía cualquier comando AT al módulo, devolviendo error si falla.
   * Utiliza internamente la lógica del LsmAtParser.
   *
   * @param cmd El comando AT exacto (ej. "ATZ" o "AT+MODE=1").
   * @param timeoutMs Tiempo máximo bloqueante de espera (por defecto 2000 ms).
   * @param retries Número de reintentos en caso de error temporal o timeout (por defecto _maxRetries).
   * @return AtError::OK si el módulo respondió OK de forma síncrona, otro Enum si no.
   */
  AtError sendCommand(const char* cmd, uint32_t timeoutMs = 2000, int8_t retries = -1);

  /**
   * @brief Envía un comando AT y captura su respuesta en un bloque de texto.
   * Ideal para getters (ej. "AT+DEUI=?", "AT+BAT=?").
   *
   * @param cmd El comando AT exacto.
   * @param outBuffer El buffer donde el usuario quiere guardar la respuesta (sin el comando de eco o el OK).
   * @param outSize Capacidad del outBuffer.
   * @param expectedTag Si se espera que el módulo prefije la respuesta con un tag (ej. "APP_VERSION:" o "DevEui:"), esto lo filtra. Usa nullptr para
   * cadena en bruto.
   * @param timeoutMs Tiempo de espera.
   * @param retries Número de reintentos en caso de error temporal o timeout (por defecto _maxRetries).
   * @return AtError::OK si se completó y se copió algo en outBuffer.
   */
  AtError sendCommandWithResponse(const char* cmd, char* outBuffer, size_t outSize, const char* expectedTag = nullptr, uint32_t timeoutMs = 2000,
                                  int8_t retries = -1);

  // =========================================================================
  // COMANDOS AT BÁSICOS / GENERALES
  // =========================================================================

  /**
   * @brief Ejecuta un factory reset devolviendo al módulo a su estado de fábrica.
   * Cuidado: Esto borra todas las llaves LoRaWAN/Sigfox escritas.
   * @return true si el módulo responde afirmativamente.
   */
  bool factoryReset();

  /**
   * @brief Realiza un reinicio por software (Comando ATZ).
   * @return true si el reinicio y boot alert fueron exitosos.
   */
  bool softwareReset();

  /**
   * @brief Realiza un reinicio por hardware usando el pin de reset.
   * @return true si el reinicio y boot alert fueron exitosos.
   */
  bool hardwareReset();

  /**
   * @brief Ejecuta el protocolo de recuperación. Primero Soft-Reset (ATZ), luego Hard-Reset si hay pin.
   * @return true si logró recuperar el módulo y recibir el Boot Alert.
   */
  bool recoverModule();

  // =========================================================================
  // SETTERS
  // =========================================================================

  /**
   * @brief Configura el baudrate del módulo (por defecto suele ser 9600).
   * @param baudrate 9600, 19200, 38400, 57600, 115200.
   * @return true si tuvo éxito.
   */
  bool setBaudrate(uint32_t baudrate);

  /**
   * @brief Configura el nivel de verbosidad del módulo.
   * @param level El nivel de log deseado.
   * @return true si tuvo éxito.
   */
  bool setVerboseLevel(uint8_t level);

  /**
   * @brief Configura el modo de comunicación.
   * @param mode LsmMode::SIGFOX o LsmMode::LORAWAN
   * @return true si tuvo éxito.
   */
  bool setMode(LsmMode mode);

  /**
   * @brief Configura el pin de reset del módulo (si es que se usa alguno).
   * @param pin El número de pin GPIO conectado al reset del módulo.
   */
  void setResetPin(int pin);

  /**
   * @brief Establece el número máximo de reintentos para un comando en caso de error temporal o timeout.
   * @param retries Número de reintentos (mínimo 1).
   */
  void setMaxRetries(int retries);

  // =========================================================================
  // ESTADO Y SINCRONIZACIÓN NATIVA
  // =========================================================================

  /**
   * @brief Indica si el módulo está actualmente unido a la red (JOINED).
   */
  bool isJoined() const;

  /**
   * @brief Espera de forma síncrona/bloqueante a que ocurra un evento asíncrono.
   * Útil para envolver comandos como Join o Send.
   * @param bitsToWaitFor Máscara de bits (ej. LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL)
   * @param timeoutMs Tiempo de espera máximo
   * @param clearOnExit Si debe limpiar los bits al salir
   * @return Los bits que desencadenaron la salida, o 0 si fue timeout.
   */
  uint32_t waitForEvent(uint32_t bitsToWaitFor, uint32_t timeoutMs, bool clearOnExit = true);

  /**
   * @brief Limpia los bits del EventGroup interno manualmente.
   */
  void clearEvents(uint32_t bitsToClear);

  // =========================================================================
  // GETTERS
  // =========================================================================

  /**
   * @brief Obtiene el voltaje de la batería en mV.
   * @return El voltaje >= 0 (ej. 3300 para 3.3V) o -1 si hubo error.
   */
  int getBattery();

  /**
   * @brief Obtiene la versión del firmware del módulo "APP_VERSION".
   * @param outBuffer Buffer donde copiar el string (ej. "V1.0.4").
   * @param size Capacidad máxima del buffer.
   * @return true si tiene éxito.
   */
  bool getVersion(char* outBuffer, size_t size);

  /**
   * @brief Obtiene el tiempo local del módulo (ej. "2024-01-01 12:00:00").
   * @param timeinfo Puntero a la estructura estandar tm donde se guardará el resultado
   * @return true si se obtuvo y parseó correctamente.
   */
  bool getLocalTime(struct tm* timeinfo);

  /**
   * @brief Obtiene el baudrate actual configurado en el módulo.
   * @return El baudrate (ej. 9600) o -1 si falla.
   */
  int getBaudrate();

  /**
   * @brief Obtiene la versión del stack de Sigfox.
   */
  bool getSigfoxVersion(char* buffer, size_t size);

  /**
   * @brief Obtiene el tipo de dispositivo detectado (ej LSM100A).
   */
  LsmModuleType getDeviceType() const;

  // =========================================================================
  // SUB-MÓDULOS (APIs LORAWAN Y SIGFOX)
  // =========================================================================
  LSM1x0A_LoRaWAN lorawan;

private:
  UartDriver*       _driver;
  LSM1x0A_AtParser* _parser;
  bool              _initialized;
  bool              _isJoined; 

  int _resetPin;
  int _maxRetries;

  // Sincronización asíncrona
  EventGroupHandle_t _syncEventGroup;

  // Callback del usuario
  AtEventCallback _userCallback;
  void*           _userCtx;

  // Interceptor
  static void internalEventCallback(const char* type, const char* payload, void* ctx);
  void handleEvent(const char* type, const char* payload);
};

#endif // LSM1X0A_CONTROLLER_H
