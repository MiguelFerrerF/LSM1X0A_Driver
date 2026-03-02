#ifndef LSM1X0A_SIGFOX_H
#define LSM1X0A_SIGFOX_H

#include "LSM1x0A_Types.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>
// Forward declaration of the main controller to avoid circular includes
class LSM1x0A_Controller;

/**
 * @defgroup Sigfox_API LSM1x0A Sigfox API
 * @brief Component dedicated to managing the module's Sigfox MAC and transmission operations.
 * @{
 */

/**
 * @class LSM1x0A_Sigfox
 * @brief Submodule handling all Sigfox-specific AT commands and operations.
 *
 * Provides getters, setters, and transmission functions compliant with the Sigfox protocol.
 */
class LSM1x0A_Sigfox
{
public:
  /**
   * @brief Constructs the Sigfox API submodule.
   * @param controller Pointer to the main generic controller instance.
   */
  explicit LSM1x0A_Sigfox(LSM1x0A_Controller* controller);

  // =========================================================================
  // Getters
  // =========================================================================

  /**
   * @brief Reads the Sigfox Device ID.
   * @param outBuffer A character array to hold the ID string.
   * @param maxLen Maximum size of the buffer (should be at least 12).
   * @return true if successfully extracted.
   */
  bool getDeviceID(char* outBuffer, size_t maxLen);

  /**
   * @brief Reads the Sigfox Initial Porting Authorization Code (PAC).
   * @param outBuffer A character array to hold the PAC string.
   * @param maxLen Maximum size of the buffer (should be at least 20).
   * @return true if successfully extracted.
   */
  bool getInitialPAC(char* outBuffer, size_t maxLen);

  /**
   * @brief Checks the currently active Regional Macro-Channel (RC).
   * @return LsmRCChannel active enum.
   */
  LsmRCChannel getRcChannel();

  /**
   * @brief Gets the configured TX power for Sigfox.
   * @return Power in dBm.
   */
  int getRadioPower();

  /**
   * @brief Checks if Public Key mode is active.
   * @return true if public key mode is used.
   */
  bool getPublicKeyMode();

  /**
   * @brief Checks if Payload Encryption is enabled.
   * @return true if enabled.
   */
  bool getPayloadEncryption();

  /**
   * @brief Gets the last RSSI received by the module from a Sigfox Downlink.
   * @return RSSI in dBm.
   */
  int16_t getLastRxRSSI() const;

  /**
   * @brief Gets the timestamp of the last Sigfox Downlink received.
   * @param timeinfo Pointer to a tm struct.
   * @return true if successful and a downlink was previously captured.
   */
  bool getLastDownlinkTime(struct tm* timeinfo) const;

  // =========================================================================
  // Setters
  // =========================================================================

  /**
   * @brief Configures the Regional Macro-Channel (RC).
   * @param rc Target RC Channel.
   * @return true if operation was successful.
   */
  bool setRcChannel(LsmRCChannel rc);

  /**
   * @brief Configures the Radio TX Power for Sigfox.
   * @param power_dBm Target transmission power in dBm.
   * @return true if setting was successful.
   */
  bool setRadioPower(int power_dBm);

  /**
   * @brief Toggles the use of the Public Key.
   * @param public_key true to use public key for tests.
   * @return true if successful.
   */
  bool setPublicKeyMode(bool public_key);

  /**
   * @brief Toggles Payload Encryption.
   * @param active true to encrypt AT payloads.
   * @return true if successful.
   */
  bool setPayloadEncryption(bool active);

  // =========================================================================
  // Operations (TX)
  // =========================================================================

  /**
   * @brief Simulates a join request via Sigfox network by sending a 1-bit frame with downlink expected.
   * @return true if a downlink response was successfully captured.
   */
  bool join();

  /**
   * @brief Sends a single bit of information. Frame Type 0.
   * @param bit boolean truth value to send.
   * @param downlink Requests a downlink reception window from the server.
   * @param txRepeat Number of times to repeat the message.
   * @return true if message was successfully acknowledged by the local modem.
   */
  bool sendBit(bool bit, bool downlink = false, uint8_t txRepeat = 2);

  /**
   * @brief Sends standard ASCII frame.
   * @param text String up to 12 characters.
   * @param downlink Requests downlink from network.
   * @param txRepeat Number of repetitions.
   * @return true upon success.
   */
  bool sendFrame(const char* text, bool downlink = false, uint8_t txRepeat = 2);

  /**
   * @brief Sends raw hex payload.
   * @param payload Byte array (up to 12 bytes).
   * @param len Length.
   * @param downlink Requests downlink.
   * @param txRepeat Number of repetitions.
   * @return true upon success.
   */
  bool sendHexFrame(const uint8_t* payload, size_t len, bool downlink = false, uint8_t txRepeat = 2);

  /**
   * @brief Sends an Out-Of-Band (OOB) administrative message to the Sigfox Operator.
   * @return true if successful.
   */
  bool sendOutOfBand();

  // =========================================================================
  // Tests and Scan Modes
  // =========================================================================

  /**
   * @brief Continuous Wave (CW) test.
   * @param freqHz Frequency in Hz. Optional.
   * @return true if successful.
   */
  bool testContinuousWave(long freqHz = 0);

  /**
   * @brief BPSK pseudo-random PRBS9 test.
   * @param freqHz Frequency in Hz. Optional.
   * @param bitrate Bitrate. Optional.
   * @return true if successful.
   */
  bool testPRBS9(long freqHz = 0, int bitrate = 0);

  /**
   * @brief Monarch pattern scan.
   * @param timeoutSecs Duration of the scan in seconds.
   * @return true if successful.
   */
  bool testMonarchScan(int timeoutSecs = 300);

  /**
   * @brief Generic Sigfox Test Mode.
   * @param mode Test mode number (0 to 12).
   * @return true if successful.
   */
  bool testMode(int mode);

  /**
   * @brief Starts Local Loop listening.
   * @return true if successful.
   */
  bool testListenLoop();

  /**
   * @brief Sends a Local Loop TX packet.
   * @return true if successful.
   */
  bool testSendLoop();

  /**
   * @brief Sends P2P data.
   * @return true if successful.
   */
  bool testSendP2P();

  /**
   * @brief Receives P2P data.
   * @return true if successful.
   */
  bool testReceiveP2P();

  // =========================================================================
  // CACHE & STATE API
  // =========================================================================

  /**
   * @brief Invalidates local volatile cache (forces queries to module hardware on next get).
   */
  void clearCache();

  /**
   * @brief Downloads variables from the module and caches them locally.
   * @return true if cache loaded successfully.
   */
  bool loadConfigFromModule();

  /**
   * @brief Uploads locally defined cache (e.g. from user code) to the hardware.
   * @return true if write succeded.
   */
  bool restoreConfig();

private:
  LSM1x0A_Controller* _controller;
  
  // Cache variables instead of LsmSigfoxConfig struct
  char         _cachedDeviceID[12] = {0};
  char         _cachedPAC[20]      = {0};
  LsmRCChannel _cachedRcChannel    = LsmRCChannel::RC_UNKNOWN;
  int          _cachedRadioPower   = -1;
  int8_t       _cachedPublicKey    = -1; // -1 unknowns, 0 false, 1 true
  int8_t       _cachedEncrypt      = -1;

  int16_t      _lastRxRSSI         = 0;
  time_t       _lastDownlinkTime   = 0;

  bool parseSigfoxDownlink(const char* rxBuffer);
};

/** @} */

#endif // LSM1X0A_SIGFOX_H
