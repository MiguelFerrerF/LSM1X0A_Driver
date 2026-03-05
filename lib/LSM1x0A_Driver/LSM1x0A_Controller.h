#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "api/LSM1x0A_LoRaWAN.h"
#include "api/LSM1x0A_Sigfox.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

// Definition of bits for the internal EventGroup synchronization
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
 * This class provides a simple interface for the user, abstracting
 * the underlying UART driver and AT parser. It allows initializing
 * communication, managing event callbacks, and sending general commands
 * obtaining their responses easily.
 */
typedef void (*LsmRxCallback)(void* ctx, const char* payload);

class LSM1x0A_Controller
{
public:
  // =========================================================================
  // CONSTRUCTORS AND DESTRUCTORS
  // =========================================================================

  /**
   * @brief Default constructor.
   * Creates the internal instances of UartDriver and LSM1x0A_AtParser,
   * ready to be initialized via begin().
   */
  LSM1x0A_Controller();

  /**
   * @brief Destructor.
   * Releases resources and de-initializes internal components.
   */
  ~LSM1x0A_Controller();

  // =========================================================================
  // INITIALIZATION AND COMMUNICATION CONTROL
  // =========================================================================

  /**
   * @brief Initializes the driver, AT parser, and internal EventGroup.
   * @return true if successful.
   */
  bool begin();

  /**
   * @brief Stops the hardware and unbinds event listeners.
   */
  void end();

  /**
   * @brief Wakes up the module and tries to synchronize its initial state.
   * Sends basic AT commands or 'Wake-Up' characters.

   * @return true if the module responds "OK" to the basic command.
   */
  bool wakeUp();
  /**
   * @brief Sends any AT command to the module, returning error if it fails.
   * Internally uses the LsmAtParser logic.

   * @param cmd The exact AT command (e.g. "ATZ" or "AT+MODE=1").
   * @param timeoutMs Maximum blocking wait time (default 2000 ms).
   * @param retries Number of retries in case of temporary error or timeout (default _maxRetries).
   * @return AtError::OK if the module responded OK synchronously, another Enum if not.
   */
  AtError sendCommand(const char* cmd, uint32_t timeoutMs = 2000, int8_t retries = -1);

  /**
   * @brief Sends an AT command and captures its response in a text block.
   * Ideal for getters (e.g. "AT+DEUI=?", "AT+BAT=?").

   * @param cmd The exact AT command.
   * @param outBuffer The buffer where the user wants to store the response (without echo command or OK).
   * @param outSize Capacity of outBuffer.
   * @param expectedTag If the module is expected to prefix the response with a tag (e.g. "APP_VERSION:" or "DevEui:"), this filters it. Use nullptr
   for raw string.
   * @param timeoutMs Wait time.
   * @param retries Number of retries in case of temporary error or timeout (default _maxRetries).
   * @return AtError::OK if completed and something was copied to outBuffer.
   */
  AtError sendCommandWithResponse(const char* cmd, char* outBuffer, size_t outSize, const char* expectedTag = nullptr, uint32_t timeoutMs = 2000,
                                  int8_t retries = -1);

  // =========================================================================
  // BASIC / GENERAL AT COMMANDS
  // =========================================================================

  /**
   * @brief Executes a factory reset returning the module to its factory state.
   * Warning: This erases all written LoRaWAN/Sigfox keys.
   * @return true if the module responds affirmatively.
   */
  bool factoryReset();

  /**
   * @brief Performs a software reset (ATZ Command).
   * @return true if the reset and boot alert were successful.
   */
  bool softwareReset();

  /**
   * @brief Performs a hardware reset using the reset pin.
   * @return true if the reset and boot alert were successful.
   */
  bool hardwareReset();

  /**
   * @brief Executes the recovery protocol. First Soft-Reset (ATZ), then Hard-Reset if there is a pin.
   * @return true if the module was recovered and Boot Alert received.
   */
  bool recoverModule();

  /**
   * @brief Synchronizes the internal RAM cache by reading the key operational parameters from the module's firmware.
   * Useful after first boot to align hardware state with software routine.
   * @return true if communication with the module was successful.
   */
  bool syncConfigToCache();

  // =========================================================================
  // SETTERS
  // =========================================================================

  /**
   * @brief Sets the module's verbosity level.
   * @param level Desired log level.
   * @return true if successful.
   */
  bool setVerboseLevel(uint8_t level);

  /**
   * @brief Sets the communication mode.
   * @param mode LsmMode::SIGFOX or LsmMode::LORAWAN
   * @return true if successful.
   */
  bool setMode(LsmMode mode);

  /**
   * @brief Sets the module's reset pin (if any is used).
   * @param pin The GPIO pin number connected to the module's reset.
   */
  void setResetPin(int pin);

  /**
   * @brief Sets the maximum number of retries for a command in case of temporary error or timeout.
   * @param retries Number of retries (minimum 1).
   */
  void setMaxRetries(int retries);

  /**
   * @brief Configures the callback and level for the external logger system.
   * @param callback Function to be called with log events
   * @param runtimeLevel The maximum severity level that will trigger the callback (e.g. VERBOSE, INFO).
   */
  void setLogCallback(LsmLogCallback callback, LsmLogLevel runtimeLevel = LsmLogLevel::VERBOSE);

  /**
   * @brief Configures a dedicated callback for receiving downlink data (RX_DATA).
   * @param callback Function to be called when payload is received.
   * @param ctx Optional context pointer passed to the callback.
   */
  void setRxCallback(LsmRxCallback callback, void* ctx = nullptr);

  // =========================================================================
  // NATIVE STATE AND SYNCHRONIZATION
  // =========================================================================

  /**
   * @brief Synchronously/blockingly waits for an asynchronous event to occur.
   * Useful for wrapping commands like Join or Send.
   * @param bitsToWaitFor Bitmask (e.g. LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL)
   * @param timeoutMs Maximum wait time
   * @param clearOnExit Whether to clear the bits on exit
   * @return The bits that triggered the exit, or 0 if timeout.
   */
  uint32_t waitForEvent(uint32_t bitsToWaitFor, uint32_t timeoutMs, bool clearOnExit = true);

  /**
   * @brief Manually clears bits from the internal EventGroup.
   */
  void clearEvents(uint32_t bitsToClear);

  // =========================================================================
  // GETTERS
  // =========================================================================

  /**
   * @brief Gets the battery voltage in mV.
   * @return Voltage >= 0 (e.g. 3300 for 3.3V) or -1 if error.
   */
  int getBattery();

  /**
   * @brief Gets the module firmware version "APP_VERSION".
   * @param outBuffer Buffer to copy the string (e.g. "V1.0.4").
   * @param size Maximum buffer capacity.
   * @return true if successful.
   */
  bool getVersion(char* outBuffer, size_t size);

  /**
   * @brief Gets the Sigfox stack version.
   */
  bool getSigfoxVersion(char* buffer, size_t size);

  /**
   * @brief Gets the detected device type (e.g. LSM100A).
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
  // SUBMODULES (LORAWAN AND SIGFOX APIs)
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

  LsmRxCallback _rxCallback = nullptr;
  void*         _rxCtx      = nullptr;

  int _resetPin   = LSM1X0A_RESET_PIN;
  int _maxRetries = DEFAULT_MAX_RETRIES;

  // Last received network / radio metadata
  int _lastRssi  = 0;
  int _lastSnr   = 0;
  int _lastDmodm = 0;
  int _lastGwn   = 0;

  // Temporary buffers for extracting configuration lists
  uint16_t _tempMaskBuffer[6] = {0};
  int      _tempMaskCount     = 0;

  // Asynchronous synchronization
  EventGroupHandle_t _syncEventGroup = nullptr;

  // Interceptor
  static void internalEventCallback(const char* type, const char* payload, void* ctx);
  void        handleEvent(const char* type, const char* payload);

  // =========================================================================
  // CONFIGURATION AND STATE RECOVERY METHODS
  // =========================================================================

  /**
   * @brief  Recovers the module configuration after an unexpected reset, re-applying the saved parameters.
   * This is useful to maintain module state consistency even if communication is lost or the module resets itself.
   * @return true if configuration was recovered (e.g. re-applied key parameters like DevEUI, AppKey, Band, Class, etc.) and the module responded to
   * configuration commands, false if not (e.g. did not respond to configuration commands or there was a critical error).
   */
  bool recoverModuleConfig();

  /**
   * @brief Recovers the module's operational state after an unexpected reset, re-executing actions like Join if the module was joined before the
   * reset.
   *  @return true if operational state was recovered (e.g. re-joined if it was joined before), false if not (e.g. did not respond to configuration
   * commands or could not rejoin).
   */
  bool recoverModuleState();
};

/** @} */ // end of Hardware_Controller group

#endif // LSM1X0A_CONTROLLER_H
