#ifndef LSM1X0A_LORAWAN_H
#define LSM1X0A_LORAWAN_H

#include "../LSM1x0A_Types.h"
#include <stddef.h>

// Forward declaration para inyección de dependencias
class LSM1x0A_Controller;

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
  LSM1x0A_LoRaWAN(LSM1x0A_Controller* controller);

  // 1. Gestión de Llaves, IDs y EUIs
  bool setDevEUI(const char* devEui);
  bool setAppEUI(const char* appEui);
  bool setAppKey(const char* appKey);
  bool setNwkKey(const char* nwkKey);
  bool setDevAddr(const char* devAddr);
  bool setAppSKey(const char* appSKey);
  bool setNwkSKey(const char* nwkSKey);
  bool setNwkID(int nwkId);

  // 2. Configuración de Red Mac (LoRaWAN)
  bool setBand(LsmBand band);
  bool setClass(LsmClass lorawanClass);
  bool setADR(bool enabled);
  bool setDataRate(LsmDataRate dr);
  bool setDutyCycle(bool enabled);
  bool setRx1Delay(int delayMs);
  bool setRx2Delay(int delayMs);
  bool setRx2DataRate(LsmDataRate dr);
  bool setRx2Frequency(int freqHz);
  bool setJoin1Delay(int delayMs);
  bool setJoin2Delay(int delayMs);
  bool setTxPower(LsmTxPower power);
  bool setPingSlot(LsmPingSlot slot);
  bool setNetworkType(LsmNetworkType type);
  bool setConfirmRetry(int retries);
  bool setUnconfirmRetry(int retries);
  bool setChannelMask(LsmBand band, uint16_t subBandMask = 0xFFFF);
  bool setDevNonce(int nonce);
  bool resetDevNonce();

  // 3. Modos Operacionales y Red (LoRaWAN)
  bool setJoinMode(LsmJoinMode mode);
  bool join(LsmJoinMode joinMode, uint32_t timeoutMs = 60000);
  bool sendData(uint8_t port, const char* data, bool confirmed = false, uint32_t timeoutMs = 0);
  bool requestLinkCheck();

  // 4. Modo de Pruebas y Certificación (RF Test)
  bool setRfTestConfig(long freqHz, int power, int bw, int sf_dr, int cr, int lna);
  bool startTxTest(int packets);
  bool startRxTest(int packets);
  bool startTxTone();
  bool startRxRssiTone();
  bool stopTest();
  bool setCertificationMode(LsmJoinMode mode);
  bool startTxHoppingTest(long fStart, long fStop, long fDelta, int packets);
  bool startContinuousModulationTx();
  bool startContinuousModulationRx();
  bool sendCertificationPacket();
  bool setP2pConfig(const char* configString);
  bool sendP2pData(const char* hexData);
  bool receiveP2pData(int timeoutMs);

  // 5. Getters de Configuración (LoRaWAN)
  bool getDevEUI(char* outBuffer, size_t size);
  bool getAppEUI(char* outBuffer, size_t size);
  bool getAppKey(char* outBuffer, size_t size);
  bool getNwkKey(char* outBuffer, size_t size);
  bool getDevAddr(char* outBuffer, size_t size);
  bool getAppSKey(char* outBuffer, size_t size);
  bool getNwkSKey(char* outBuffer, size_t size);
  int  getNwkID();

  int            getDevNonce();
  int            getADR();
  LsmDataRate    getDataRate();
  LsmTxPower     getTxPower();
  LsmBand        getBand();
  LsmClass       getClass();
  int            getDutyCycle();
  int            getJoin1Delay();
  int            getJoin2Delay();
  int            getRx1Delay();
  int            getRx2Delay();
  LsmDataRate    getRx2DataRate();
  long           getRx2Frequency();
  LsmPingSlot    getPingSlot();
  int            getConfirmRetry();
  int            getUnconfirmRetry();
  LsmNetworkType getNetworkType();
  bool           getChannelMask(uint16_t* outMasks, size_t* outArraySize);

// 6. Gestión de Estado y Recuperación
  bool restoreConfig();
  bool loadConfigFromModule();
  void clearCache();

  // 7. Estado de Conexión y Recuperación
  bool isJoined() const;
  void setJoined(bool joined);
  bool recoverConnection(int maxRetries);

private:
  LSM1x0A_Controller* _controller = nullptr;
  LsmJoinMode         _joinMode   = LsmJoinMode::OTAA;
  bool                _isJoined   = false;
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
  uint16_t    _cachedSubBandMask    = 0xFFFF;
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

#endif // LSM1X0A_LORAWAN_H
