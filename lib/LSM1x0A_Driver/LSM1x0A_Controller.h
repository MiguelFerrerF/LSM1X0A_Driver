#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include "api/LSM1x0A_LoRaWAN.h"
#include "api/LSM1x0A_Sigfox.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <Arduino.h>
#include <time.h>

// Definición de bits para el EventGroup interno de sincronización
#define LSM_EVT_JOIN_SUCCESS (1 << 0)
#define LSM_EVT_JOIN_FAIL (1 << 1)
#define LSM_EVT_TX_SUCCESS (1 << 2)
#define LSM_EVT_TX_FAIL (1 << 3)
#define LSM_EVT_RX_DATA (1 << 4)
#define LSM_EVT_RX_TIMEOUT (1 << 5)
#define LSM_EVT_LINK_CHECK_ANS (1 << 6)

/**
 * @defgroup Hardware_Controller LSM1x0A Hardware & Controller
 * @brief Core system interface, asynchronous event routing, UART management, and generic AT communication.
 * @{
 */

/**
 * @class LSM1x0A_Controller
 * @brief Main class for coordinating the LSM100A/LSM110A module.
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

  /**
   * @brief Sincroniza la caché RAM interna leyendo los parámetros clave operativos del firmware del módulo.
   * Útil tras arrancar por primera vez para alinear el estado del hardware con la rutina software.
   * @return true si tuvo éxito comunicándose con el módulo.
   */
  bool syncConfigToCache();

  // =========================================================================
  // SETTERS
  // =========================================================================

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
   * @brief Obtiene la versión del stack de Sigfox.
   */
  bool getSigfoxVersion(char* buffer, size_t size);

  /**
   * @brief Obtiene el tipo de dispositivo detectado (ej LSM100A).
   */
  LsmModuleType getDeviceType() const;

  /**
   * @brief Getters para los últimos metadatos de radio recibidos.
   */
  /** @brief Gets the last RSSI received by the module. */
  int getLastRssi() const
  {
    return _lastRssi;
  }

  /** @brief Gets the last SNR received by the module. */
  int getLastSnr() const
  {
    return _lastSnr;
  }

  /** @brief Gets the last Demodulation Margin received by the module. */
  int getLastDemodMargin() const
  {
    return _lastDmodm;
  }

  /** @brief Gets the last Gateway Count reported by LinkCheck. */
  int getLastNbGateways() const
  {
    return _lastGwn;
  }

  // =========================================================================
  // SUB-MÓDULOS (APIs LORAWAN Y SIGFOX)
  // =========================================================================

  /** @brief LoRaWAN specific configuration and transmission API context. */
  LSM1x0A_LoRaWAN lorawan;

  /** @brief Sigfox specific configuration and transmission API context. */
  LSM1x0A_Sigfox sigfox;

  /** @brief Internal helper to return the static temporary Channel Mask buffer pointer. */
  const uint16_t* getTempMaskBuffer() const
  {
    return _tempMaskBuffer;
  }

  /** @brief Internal helper to return the temporary Channel Mask active element count. */
  int getTempMaskCount() const
  {
    return _tempMaskCount;
  }

  /** @brief Internal helper to zero-out the static temporary Channel Mask buffer. */
  void resetTempMaskBuffer()
  {
    _tempMaskCount = 0;
    memset(_tempMaskBuffer, 0, sizeof(_tempMaskBuffer));
  }

private:
  UartDriver*       _driver      = nullptr;
  LSM1x0A_AtParser* _parser      = nullptr;
  bool              _initialized = false;
  LsmMode           _currentMode = LsmMode::LORAWAN;

  int _resetPin   = LSM1X0A_RESET_PIN;
  int _maxRetries = DEFAULT_MAX_RETRIES;

  // Últimos metadatos de red / radio recibidos
  int _lastRssi  = 0;
  int _lastSnr   = 0;
  int _lastDmodm = 0;
  int _lastGwn   = 0;

  // Buffers temporales para extracciones de listas de configuración
  uint16_t _tempMaskBuffer[6] = {0};
  int      _tempMaskCount     = 0;

  // Sincronización asíncrona
  EventGroupHandle_t _syncEventGroup = nullptr;

  // Callback del usuario
  AtEventCallback _userCallback = nullptr;
  void*           _userCtx      = nullptr;

  // Interceptor
  static void internalEventCallback(const char* type, const char* payload, void* ctx);
  void        handleEvent(const char* type, const char* payload);

  // =========================================================================
  // MÉTODOS DE RECUPERACIÓN DE CONFIGURACIÓN Y ESTADO
  // =========================================================================

  /**
   * @brief  Recupera la configuración del módulo después de un reinicio inesperado, re-aplicando los parámetros guardados.
   * Esto es útil para mantener la consistencia del estado del módulo incluso si se pierde la comunicación o el módulo se reinicia por sí solo.
   * @return true si logró recuperar la configuración (ej. re-aplicar parámetros clave como DevEUI, AppKey, Band, Class, etc.) y el módulo respondió a
   * los comandos de configuración, false si no pudo recuperarla (ej. no respondió a los comandos de configuración o hubo un error crítico).
   */
  bool recoverModuleConfig();

  /**
   * @brief Recupera el estado operativo del módulo después de un reinicio inesperado, re-ejecutando acciones como Join si el módulo estaba unido
   * antes del reinicio.
   *  @return true si logró recuperar el estado operativo (ej. volver a unirlo si estaba unido antes), false si no pudo recuperarlo (ej. no respondió
   * a los comandos de configuración o no se pudo unir de nuevo).
   */
  bool recoverModuleState();
};

/** @} */ // end of Hardware_Controller group

#endif // LSM1X0A_CONTROLLER_H
