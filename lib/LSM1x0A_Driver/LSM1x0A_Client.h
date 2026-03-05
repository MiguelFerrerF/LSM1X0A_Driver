#ifndef LSM1X0A_CLIENT_H
#define LSM1X0A_CLIENT_H

#include "LSM1x0A_Controller.h"

/**
 * @defgroup Application_Client LSM1x0A High-Level Client
 * @brief Simplified API wrapping the controller for rapid end-user application development.
 * @{
 */

/**
 * @class LSM1x0A_Client
 * @brief High-level orchestration client for the LSM100A/LSM110A module.
 *
 * This class wraps an internal instance of `LSM1x0A_Controller` and exposes
 * an ultra-simplified interface for setting up and sending data via LoRaWAN or Sigfox,
 * managing all the required initialization, recovery, and synchronization routines internally.
 */
/**
 * @brief Downlink callback definition.
 * @param port Application port (for LoRaWAN).
 * @param payload Binary payload received.
 * @param size Size in bytes.
 */
typedef void (*LsmDownlinkCallback)(uint8_t port, const uint8_t* payload, size_t size);

class LSM1x0A_Client
{
public:
  /**
   * @brief Construct a new Client object.
   */
  LSM1x0A_Client();

  /**
   * @brief Destroy the Client object. Stops processes and deletes internal controller gracefully.
   */
  ~LSM1x0A_Client();

  // =========================================================================
  // LIFE-CYCLE AND SETUP
  // =========================================================================

  /**
   * @brief Initializes the driver, AT parser, and attempts to wake up the module.
   * @param logCb Optional callback to receive driver log messages.
   * @return true if communication and initialization were successful.
   */
  bool begin(LsmLogCallback logCb = nullptr);

  /**
   * @brief Pings the module or checks the battery to verify communication is still active.
   * @return true if the module responds.
   */
  bool checkConnection();

  // =========================================================================
  // NETWORK CONFIGURATION
  // =========================================================================

  /**
   * @brief Configures the module for LoRaWAN using OTAA (Over The Air Activation).
   * Note: This only sets the parameters. You must still call `joinNetwork()`.
   * @param band The operating LoRaWAN band (e.g. LsmBand::EU868).
   * @param appEui 16-char hex string.
   * @param appKey 32-char hex string.
   * @param devClass Device Class (default CLASS_A).
   * @return true if all parameters were set successfully.
   */
  bool setupLoRaWAN_OTAA(LsmBand band = LsmBand::BAND_UNKNOWN, const char* appEui = nullptr, const char* appKey = nullptr,
                         LsmClass devClass = LsmClass::CLASS_A);

  /**
   * @brief Configures the module for LoRaWAN using ABP (Activation By Personalization).
   * Note: This only sets the parameters. You must still call `joinNetwork()`.
   * @param band The operating LoRaWAN band.
   * @param devAddr 8-char hex string.
   * @param nwkSKey 32-char hex string.
   * @param appSKey 32-char hex string.
   * @param devClass Device Class (default CLASS_A).
   * @return true if all parameters were set successfully.
   */
  bool setupLoRaWAN_ABP(LsmBand band, const char* devAddr, const char* nwkSKey, const char* appSKey, LsmClass devClass = LsmClass::CLASS_A);

  /**
   * @brief Configures the module for Sigfox.
   * @param rcZone The Radio Configuration Zone (e.g. LsmRCChannel::RC1 for Europe).
   * @return true if configuration was successful.
   */
  bool setupSigfox(LsmRCChannel rcZone);

  // =========================================================================
  // NETWORK EXECUTION
  // =========================================================================

  /**
   * @brief Executes the connection procedure to the network.
   * For LoRaWAN: Executes OTAA Join procedure or applies ABP settings.
   * For Sigfox: Sends a dummy payload or prepares the module for network.
   * @return true if connection or setup was successful.
   */
  bool joinNetwork();

  /**
   * @brief Checks if the module is successfully joined to the LoRaWAN network.
   * @return true if joined, false otherwise. (Always true for Sigfox once configured).
   */
  bool isJoined();

  // =========================================================================
  // DATA TRANSMISSION
  // =========================================================================

  /**
   * @brief Sends maximum length data payload to the currently configured network.
   * Routing (LoRaWAN vs Sigfox) is done automatically based on the last `setup...` command called.
   * @param payload Array containing bytes to send.
   * @param length Number of bytes to send.
   * @param requestAck Request confirmation/downlink from the network.
   * @param port LoRaWAN application port number (default 33).
   * @param enableRetries Enable application-level retries if transmission fails.
   * @param maxRetries Number of retries to attempt. If LoRaWAN and confirmed, maxing this out triggers a module recovery.
   * @return true if the transmission was successful.
   */
  bool sendPayload(const uint8_t* payload, size_t length, bool requestAck = false, uint8_t port = 33, bool enableRetries = false, uint8_t maxRetries = 3);

  /**
   * @brief Sends an ASCII string to the currently configured network.
   * @param text Null terminated string to send.
   * @param requestAck Request confirmation/downlink from the network.
   * @param port LoRaWAN application port number (default 33).
   * @param enableRetries Enable application-level retries if transmission fails.
   * @param maxRetries Number of retries to attempt. If LoRaWAN and confirmed, maxing this out triggers a module recovery.
   * @return true if the transmission was successful.
   */
  bool sendString(const char* text, bool requestAck = false, uint8_t port = 33, bool enableRetries = false, uint8_t maxRetries = 3);

  // =========================================================================
  // INFORMATION RETRIEVAL
  // =========================================================================

  /**
   * @brief Gets the device battery voltage.
   * @return Voltage in mV, or -1 if error.
   */
  int getBatteryVoltage();

  /**
   * @brief Configures the callback for receiving downlinks (RX_DATA).
   * @param callback Function called when downlink data is received.
   */
  void setDownlinkCallback(LsmDownlinkCallback callback);

  /**
   * @brief Gets the detected hardware module type.
   * @return LsmModuleType enum.
   */
  LsmModuleType getModuleType();

  /**
   * @brief Retrieves the firmware version of the module.
   * @param buffer Output buffer array.
   * @param size Size of the buffer.
   * @return true if successfully retrieved.
   */
  bool getFirmwareVersion(char* buffer, size_t size);

  /**
   * @brief Gets the last RSSI (Received Signal Strength Indicator).
   * @return RSSI in dBm.
   */
  int getLastRssi() const;

  /**
   * @brief Gets the last SNR (Signal-to-Noise Ratio).
   * @return SNR in dB.
   */
  int getLastSnr() const;

  /**
   * @brief Provides direct access to the underlying low-level controller for advanced features.
   * @return Reference to the internal LSM1x0A_Controller.
   */
  LSM1x0A_Controller& getController();

private:
  LSM1x0A_Controller* _controller;
  LsmMode             _configuredMode;
  bool                _abpConfigured;

  LsmDownlinkCallback _downlinkCallback = nullptr;
  static void _onRxData(void* ctx, const char* payload);

  // Internal helper to abstract string formatting
  bool _charsToBytes(const char* hexString, uint8_t* outputBuffer, size_t maxLen);
};

/** @} */

#endif // LSM1X0A_CLIENT_H
