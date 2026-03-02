#ifndef LSM1X0A_LORAWAN_H
#define LSM1X0A_LORAWAN_H

#include "../LSM1x0A_Types.h"
#include <stddef.h>

// Forward declaration para inyección de dependencias
class LSM1x0A_Controller;

/**
 * @defgroup LoRaWAN_API LSM1x0A LoRaWAN API
 * @brief High-level C++ wrappers for LoRaWAN Network joining, SubBands, and payload transmission.
 * @{
 */

/**
 * @class LSM1x0A_LoRaWAN
 * @brief Submódulo de configuración y operación LoRaWAN del LSM1x0A.
 *
 * Separa la lógica de comandos LoRaWAN (Setters, Getters, Operación)
 * para evitar sobrecargar el Controller principal (antipatrón God Object).
 */
class LSM1x0A_LoRaWAN
{
public:
  /**
   * @brief Intializes the LoRaWAN API wrapper.
   * @param controller Pointer to the parent `LSM1x0A_Controller` managing AT communication.
   */
  LSM1x0A_LoRaWAN(LSM1x0A_Controller* controller);

  // =========================================================================
  // 1. Keys, IDs, and EUIs Management
  // =========================================================================

  /**
   * @brief Sets the LoRaWAN Device EUI (DevEUI).
   * @param devEui A 16-character hex string representing the 8-byte Device EUI.
   * @return true if the command was accepted by the hardware, false otherwise.
   */
  bool setDevEUI(const char* devEui);

  /**
   * @brief Sets the LoRaWAN Application EUI (JoinEUI).
   * @param appEui A 16-character hex string representing the 8-byte Application EUI.
   * @return true if successful.
   */
  bool setAppEUI(const char* appEui);

  /**
   * @brief Sets the LoRaWAN Application Key (AppKey).
   * @param appKey A 32-character hex string representing the 16-byte AppKey.
   * @return true if successful.
   */
  bool setAppKey(const char* appKey);

  /**
   * @brief Sets the LoRaWAN Network Key (NwkKey).
   * @param nwkKey A 32-character hex string representing the 16-byte NwkKey.
   * @return true if successful.
   */
  bool setNwkKey(const char* nwkKey);

  /**
   * @brief Sets the LoRaWAN Device Address (DevAddr) for ABP mode.
   * @param devAddr An 8-character hex string representing the 4-byte DevAddr.
   * @return true if successful.
   */
  bool setDevAddr(const char* devAddr);

  /**
   * @brief Sets the Application Session Key (AppSKey) for ABP mode.
   * @param appSKey A 32-character hex string representing the 16-byte AppSKey.
   * @return true if successful.
   */
  bool setAppSKey(const char* appSKey);

  /**
   * @brief Sets the Network Session Key (NwkSKey) for ABP mode.
   * @param nwkSKey A 32-character hex string representing the 16-byte NwkSKey.
   * @return true if successful.
   */
  bool setNwkSKey(const char* nwkSKey);

  /**
   * @brief Sets the LoRaWAN Network ID (NetID).
   * @param nwkId The integer representation of the Network ID.
   * @return true if successful.
   */
  bool setNwkID(int nwkId);

  // =========================================================================
  // 2. MAC Network Configuration (LoRaWAN)
  // =========================================================================

  /**
   * @brief Configura el baudrate del módulo (por defecto suele ser 9600).
   * @param baudrate 9600, 19200, 38400, 57600, 115200.
   * @return true si tuvo éxito.
   */
  bool setBaudrate(uint32_t baudrate);

  /**
   * @brief Sets the LoRaWAN operational frequency band.
   * @param band The desired LsmBand (e.g., LsmBand::EU868, LsmBand::US915).
   * @return true if successful.
   */
  bool setBand(LsmBand band);

  /**
   * @brief Sets the LoRaWAN device Class (A or C).
   * @param lorawanClass The desired LsmClass representation.
   * @return true if successful.
   */
  bool setClass(LsmClass lorawanClass);

  /**
   * @brief Enables or disables Adaptive Data Rate (ADR).
   * @param enabled True to enable ADR, false to disable.
   * @return true if successful.
   */
  bool setADR(bool enabled);

  /**
   * @brief Sets the Data Rate (DR) of the module.
   * @param dr The desired LsmDataRate (DR0 to DR15).
   * @return true if successful.
   */
  bool setDataRate(LsmDataRate dr);

  /**
   * @brief Enables or disables the Duty Cycle limitation (EU regions mostly).
   * @param enabled True to enforce Duty Cycle, false otherwise.
   * @return true if successful.
   */
  bool setDutyCycle(bool enabled);

  /**
   * @brief Sets the delay for the RX1 receive window.
   * @param delayMs The delay in milliseconds.
   * @return true if successful.
   */
  bool setRx1Delay(int delayMs);

  /**
   * @brief Sets the delay for the RX2 receive window.
   * @param delayMs The delay in milliseconds.
   * @return true if successful.
   */
  bool setRx2Delay(int delayMs);

  /**
   * @brief Sets the Data Rate (DR) specifically for the RX2 window.
   * @param dr The desired LsmDataRate.
   * @return true if successful.
   */
  bool setRx2DataRate(LsmDataRate dr);

  /**
   * @brief Sets the precise frequency for the RX2 window.
   * @param freqHz Frequency in Hertz.
   * @return true if successful.
   */
  bool setRx2Frequency(int freqHz);

  /**
   * @brief Sets the delay for the Join1 receive window.
   * @param delayMs The delay in milliseconds.
   * @return true if successful.
   */
  bool setJoin1Delay(int delayMs);

  /**
   * @brief Sets the delay for the Join2 receive window.
   * @param delayMs The delay in milliseconds.
   * @return true if successful.
   */
  bool setJoin2Delay(int delayMs);

  /**
   * @brief Sets the transmission power index.
   * @param power The desired LsmTxPower index.
   * @return true if successful.
   */
  bool setTxPower(LsmTxPower power);

  /**
   * @brief Configures the Ping Slot periodicity (Class B / Custom usage).
   * @param slot The predefined LsmPingSlot periodicity index.
   * @return true if successful.
   */
  bool setPingSlot(LsmPingSlot slot);

  /**
   * @brief Sets the network type (Public vs Private).
   * @param type The desired LsmNetworkType.
   * @return true if successful.
   */
  bool setNetworkType(LsmNetworkType type);

  /**
   * @brief Configures the number of retries for Confirmed uplinks.
   * @param retries Number of retry attempts (0-7).
   * @return true if successful.
   */
  bool setConfirmRetry(int retries);

  /**
   * @brief Configures the number of repetitions for Unconfirmed uplinks.
   * @param retries Number of unconfirmed repetitions (0-15).
   * @return true if successful.
   */
  bool setUnconfirmRetry(int retries);

  /**
   * @brief Enables or disables specific sub-bands for regions supporting Channel Masks (e.g., US915, AU915).
   * @param band The current operational region/band.
   * @param subBandMask A bitmask representing which sub-bands to enable (default: ALL).
   * @return true if the mask was applied accurately to the hardware.
   */
  bool setChannelMask(LsmBand band, uint16_t subBandMask = LsmSubBand::SUB_BAND_ALL);

  /**
   * @brief Sets the DevNonce manually.
   * @param nonce The nonce integer value.
   * @return true if successful.
   */
  bool setDevNonce(int nonce);

  /**
   * @brief Resets the DevNonce counter to zero.
   * @return true if successful.
   */
  bool resetDevNonce();

  /**
   * @brief Forcibly sets the ABP frame counter value.
   * @param fcnt The desired 32-bit frame counter.
   * @return true if successful.
   */
  bool setAbpFrameCounter(uint32_t fcnt);

  // =========================================================================
  // 3. Operational Modes & Network Actions
  // =========================================================================

  /**
   * @brief Sets the join mode (OTAA vs ABP).
   * @param mode The selected LsmJoinMode.
   * @return true if successful.
   */
  bool setJoinMode(LsmJoinMode mode);

  /**
   * @brief Initiates a LoRaWAN Network Join procedure.
   *        This is a blocking function until the module fires an event or times out.
   * @param joinMode Whether to join via OTAA or ABP.
   * @param timeoutMs Maximum wait time for the join event (default 60s).
   * @return true if joined successfully, false on timeout or denial.
   */
  bool join(LsmJoinMode joinMode, uint32_t timeoutMs = 60000);

  /**
   * @brief Transmits a payload through the LoRaWAN network.
   *        Blocks until the MAC layer confirms transmission (or reception if confirmed).
   * @param port The LoRaWAN application port (1-223).
   * @param data The payload formatted as a Hexadecimal string.
   * @param confirmed True to require an ACK from the Network Server, False for "fire and forget".
   * @param timeoutMs Optional custom timeout (useful for slow DRs).
   * @return true if transmitted successfully.
   */
  bool sendData(uint8_t port, const char* data, bool confirmed = false, uint32_t timeoutMs = 0);

  /**
   * @brief Requests a Link Check MAC command on the next uplink.
   * @return true if the MAC command was successfully enqueued.
   */
  bool requestLinkCheck();

  // =========================================================================
  // 4. RF Testing & Certification Modes
  // =========================================================================

  /**
   * @brief Configures fundamental parameters for RF testing.
   * @param freqHz Frequency in Hertz.
   * @param power Power index.
   * @param bw Bandwidth (e.g., 0=125, 1=250, 2=500).
   * @param sf_dr Spreading Factor or Data Rate.
   * @param cr Coding Rate (1=4/5 ... 4=4/8).
   * @param lna Low Noise Amplifier state.
   * @return true if configuration was valid.
   */
  bool setRfTestConfig(long freqHz, int power, int bw, int sf_dr, int cr, int lna);

  /**
   * @brief Starts transmitting test packets.
   * @param packets Amount of packets to transmit.
   * @return true if started successfully.
   */
  bool startTxTest(int packets);

  /**
   * @brief Starts receiving test packets.
   * @param packets Amount of packets expected.
   * @return true if started successfully.
   */
  bool startRxTest(int packets);
  /**
   * @brief Emits a continuous Continuous Wave (CW) tone on the TX path.
   * @return true if started successfully.
   */
  bool startTxTone();

  /**
   * @brief Listens and measures RSSI on the configured frequency.
   * @return true if started successfully.
   */
  bool startRxRssiTone();

  /**
   * @brief Stops any ongoing RF Test or Tone operation.
   * @return true if stopped successfully.
   */
  bool stopTest();

  /**
   * @brief Enters LoRaWAN certification mode.
   * @param mode The join mode to certify on.
   * @return true if successful.
   */
  bool setCertificationMode(LsmJoinMode mode);

  /**
   * @brief Executes a Tx Frequency Hopping test.
   * @param fStart Start Frequency in Hz.
   * @param fStop Stop Frequency in Hz.
   * @param fDelta Step Frequency in Hz.
   * @param packets Amount of packets per step.
   * @return true if started successfully.
   */
  bool startTxHoppingTest(long fStart, long fStop, long fDelta, int packets);

  /**
   * @brief Transmits a continuous modulated LoRa signal.
   * @return true if started successfully.
   */
  bool startContinuousModulationTx();

  /**
   * @brief Receives a continuous modulated LoRa signal.
   * @return true if started successfully.
   */
  bool startContinuousModulationRx();

  /**
   * @brief Sends a predefined LoRaWAN certification packet.
   * @return true if successful.
   */
  bool sendCertificationPacket();

  /**
   * @brief Configures P2P MAC parameters bridging the LoRa modem.
   * @param configString Format dictated by the FW specification.
   * @return true if accepted.
   */
  bool setP2pConfig(const char* configString);

  /**
   * @brief Transmits raw P2P data outside the LoRaWAN stack.
   * @param hexData Hexadecimal string of the payload.
   * @return true if transmitted.
   */
  bool sendP2pData(const char* hexData);

  /**
   * @brief Switches the module into P2P RX listening mode.
   * @param timeoutMs Timeout to wait for incoming P2P packets.
   * @return true if the window opened and captured data.
   */
  bool receiveP2pData(int timeoutMs);

  // =========================================================================
  // 5. Configuration Getters (LoRaWAN)
  // =========================================================================

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
   * @brief Retrieves the Device EUI.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getDevEUI(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the Application EUI.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getAppEUI(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the Application Key.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getAppKey(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the Network Key.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getNwkKey(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the assigned DevAddr.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getDevAddr(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the Application Session Key (ABP).
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getAppSKey(char* outBuffer, size_t size);
  /**
   * @brief Retrieves the Network Session Key.
   * @param outBuffer Target character buffer.
   * @param size Maximum buffer size.
   * @return true if successfully extracted.
   */
  bool getNwkSKey(char* outBuffer, size_t size);

  /**
   * @brief Retrieves the current Network ID.
   * @return The 32-bit Network ID.
   */
  int getNwkID();

  /**
   * @brief Retrieves the current Device Nonce.
   * @return The 16-bit Device Nonce.
   */
  int getDevNonce();

  /**
   * @brief Checks if Adaptive Data Rate is enabled.
   * @return 1 if enabled, 0 if disabled, -1 if unknown.
   */
  int getADR();

  /**
   * @brief Retrieves the current operational Data Rate.
   * @return The active LsmDataRate.
   */
  LsmDataRate getDataRate();

  /**
   * @brief Retrieves the current Transmission Power index.
   * @return The active LsmTxPower.
   */
  LsmTxPower getTxPower();

  /**
   * @brief Retrieves the active regional Frequency Band.
   * @return The configured LsmBand.
   */
  LsmBand getBand();

  /**
   * @brief Retrieves the current LoRaWAN Class.
   * @return LsmClass (CLASS_A or CLASS_C).
   */
  LsmClass getClass();

  /**
   * @brief Retrieves the Duty Cycle limitation state.
   * @return 1 if enabled, 0 if disabled.
   */
  int getDutyCycle();

  /**
   * @brief Retrieves the Join1 receive window delay.
   * @return Delay in milliseconds.
   */
  int getJoin1Delay();

  /**
   * @brief Retrieves the Join2 receive window delay.
   * @return Delay in milliseconds.
   */
  int getJoin2Delay();

  /**
   * @brief Retrieves the RX1 receive window delay.
   * @return Delay in milliseconds.
   */
  int getRx1Delay();

  /**
   * @brief Retrieves the RX2 receive window delay.
   * @return Delay in milliseconds.
   */
  int getRx2Delay();

  /**
   * @brief Retrieves the Data Rate configured for the RX2 window.
   * @return The assigned LsmDataRate.
   */
  LsmDataRate getRx2DataRate();

  /**
   * @brief Retrieves the Frequency configured for the RX2 window.
   * @return Frequency in Hertz.
   */
  long getRx2Frequency();

  /**
   * @brief Retrieves the active Ping Slot periodicity index.
   * @return The configured LsmPingSlot.
   */
  LsmPingSlot getPingSlot();

  /**
   * @brief Retrieves the Confirmed uplink retries limit.
   * @return Retries count (0-7).
   */
  int getConfirmRetry();

  /**
   * @brief Retrieves the Unconfirmed uplink repetitions count.
   * @return Repetitions count (0-15).
   */
  int getUnconfirmRetry();

  /**
   * @brief Retrieves the Public/Private network mode definition.
   * @return The configured LsmNetworkType.
   */
  LsmNetworkType getNetworkType();

  /**
   * @brief Retrieves the active channel Sub-Band mask configurations.
   * @param outMasks Array pointer to store up to 6 sub-band bitmasks.
   * @param outArraySize Returns the number of masks successfully extracted based on the region.
   * @return true if successfully extracted entirely.
   */
  bool getChannelMask(uint16_t* outMasks, size_t* outArraySize);

  /**
   * @brief Retrieves the ABP Frame Counter value.
   * @return The 32-bit ABP Frame Counter.
   */
  int getAbpFrameCounter();

  // =========================================================================
  // 6. State Management & Cache Recovery
  // =========================================================================

  /**
   * @brief Pushes the RAM-cached variables back down into the hardware module.
   *        Restores the operational state after an unhandled hardware reset.
   * @return true if all cached properties were successfully pushed.
   */
  bool restoreConfig();

  /**
   * @brief Polls the module for all its properties and populates the internal RAM cache.
   *        Recommended after waking up from deep sleep to synchronize states.
   * @return true if the poll completed without major errors.
   */
  bool loadConfigFromModule();

  /**
   * @brief Wipes the internal state cache, forcing a clean slate for future read/writes.
   */
  void clearCache();

  // =========================================================================
  // 7. Connectivity Tracking
  // =========================================================================

  /**
   * @brief Validates if the device is currently Joined to the network.
   *        (Either confirmed by OTAA or statically assumed by ABP).
   * @return true if the node can transmit data.
   */
  bool isJoined() const;

  /**
   * @brief Manually overrides the internal joined state tracker.
   * @param joined True or False.
   */
  void setJoined(bool joined);

  /**
   * @brief Attempts to recover a lost connection (re-executes OTAA Join if dropped).
   * @param maxRetries Maximum attempts before giving up.
   * @return true if the connection was restored.
   */
  bool recoverConnection(int maxRetries);

private:
  LSM1x0A_Controller* _controller         = nullptr;
  LsmJoinMode         _joinMode           = LsmJoinMode::OTAA;
  bool                _isJoined           = false;
  bool                _linkCheckRequested = false;
  bool                _pendingChannelMask = false;

  // Caché de reintentos para no bombardear al módulo en cada envío asíncrono
  char        _cachedDevEui[24]     = {0};
  char        _cachedDevAddr[12]    = {0};
  char        _cachedNwkID[12]      = {0};
  int8_t      _cachedAdrEnabled     = -1;
  LsmDataRate _cachedDataRate       = LsmDataRate::DR_UNKNOWN;
  LsmTxPower  _cachedTxPower        = LsmTxPower::TP_UNKNOWN;
  LsmBand     _cachedBand           = LsmBand::BAND_UNKNOWN;
  uint16_t    _cachedSubBandMask    = LsmSubBand::SUB_BAND_ALL;
  int8_t      _cachedDutyCycle      = -1;
  int32_t     _cachedJoin1Delay     = -1;
  int32_t     _cachedJoin2Delay     = -1;
  int32_t     _cachedRx1Delay       = -1;
  int32_t     _cachedRx2Delay       = -1;
  LsmDataRate _cachedRx2DataRate    = LsmDataRate::DR_UNKNOWN;
  long        _cachedRx2Frequency   = -1;
  int         _cachedConfirmRetry   = -1;
  int         _cachedUnconfirmRetry = -1;
};

/** @} */ // end of LoRaWAN_API group

#endif // LSM1X0A_LORAWAN_H
