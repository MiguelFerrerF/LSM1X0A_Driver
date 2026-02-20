#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include "LSM1x0A_Types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "stdarg.h"
#include <vector>

typedef void (*LsmUserCallback)(const char* type, const char* payload);

class LSM1x0A_Controller
{
public:
  LSM1x0A_Controller();
  ~LSM1x0A_Controller();

  // Inicialización del controlador
  bool begin(UartDriver* driver, int resetPin = -1, LsmUserCallback callback = nullptr);

  // Método público por si el usuario quiere forzar un reset manual en runtime
  bool hardReset();
  bool softReset();

  // Método de recuperación automática en caso de fallo
  bool recoverModule();

  // Configuración de Logging
  void setLogLevel(LsmLogLevel level);

  // Configuración de reintentos automáticos en fallos de transmisión
  void setMaxTimeoutErrors(uint8_t maxErrors);

  // Métodos de Negocio
  bool lora_join(bool isOTAA);
  bool lora_sendData(uint8_t port, const char* data, bool confirmed = false);

  bool sigfox_join();
  bool sigfox_sendData(const char* data, LsmSigfoxDataType type = LsmSigfoxDataType::HEX_DATA, bool confirmed = false, uint8_t retry = 1);

  // Setters Generales
  bool setBaudrate(uint32_t baudrate);
  bool factoryReset();
  bool setVerboseLevel(uint8_t level);

  // Setters LoRaWAN
  bool setMode(bool mode);

  bool setAppEUI(const char* appEui);
  bool setDevEUI(const char* devEui);
  bool setKeys(const char* devEui = nullptr, const char* appKey = nullptr, const char* nwkKey = nullptr, const char* appEui = nullptr);
  bool setAppKey(const char* appKey);
  bool setNwkKey(const char* nwkKey);
  bool setDevAddr(const char* devAddr);
  bool setAppSKey(const char* appSKey);
  bool setNwkSKey(const char* nwkSKey);
  bool setNwkID(const char* nwkID);
  bool setDevNonce(uint32_t devNonce);
  bool setADR(bool enabled);
  bool setDataRate(LsmDataRate dr);
  bool setTxPower(LsmTxPower tp);
  bool setBand(LsmBand band);
  bool setClass(LsmClass cls);
  bool setDutyCycle(bool enabled);
  bool setJoin1Delay(int32_t delayMilis);
  bool setJoin2Delay(int32_t delayMilis);
  bool setRx1Delay(int32_t delayMilis);
  bool setRx2Delay(int32_t delayMilis);
  bool setRx2DataRate(LsmDataRate dr);
  bool setRx2Frequency(uint32_t freqHz);
  bool setPingSlot(LsmPingSlot periodicity); // Only for Class B
  bool setLinkCheck();
  bool setConfirmRetry(uint8_t count);
  bool setUnconfirmedRetry(uint8_t count);
  bool setNwkType(LsmNetworkType type);
  bool setChannelMaskBySubBand(LsmBand band, int subBand);

  bool setRCChannel(LsmRCChannel channel);
  bool setRadioPower(uint8_t power);
  bool setEncryptKey(bool privateKey);
  bool setEncryptPayload(bool enabled);

  // Getters
  bool isLSM110A();
  bool getMode();
  // Getters Generales
  uint32_t getBaudrate();
  uint8_t  getVerboseLevel();
  uint8_t  getBattery();
  void     getLocalTime(char* outBuffer, size_t outSize);
  void     getVersion(char* outBuffer, size_t outSize);
  void getAppEUI(char* outBuffer, size_t outSize);
  void getDevEUI(char* outBuffer, size_t outSize);
  void getKeys(char* devEui, size_t devEuiSize, char* appKey, size_t appKeySize, char* nwkKey, size_t nwkKeySize, char* appEui, size_t appEuiSize);
  void getAppKey(char* outBuffer, size_t outSize);
  void getNwkKey(char* outBuffer, size_t outSize);
  void getDevAddr(char* outBuffer, size_t outSize);
  void getAppSKey(char* outBuffer, size_t outSize);
  void getNwkSKey(char* outBuffer, size_t outSize);
  void getNwkID(char* outBuffer, size_t outSize);
  uint32_t       getDevNonce();
  bool           getADR();
  LsmDataRate    getDataRate();
  LsmTxPower     getTxPower();
  LsmBand        getBand();
  LsmClass       getClass();
  bool           getDutyCycle();
  int32_t        getJoin1Delay();
  int32_t        getJoin2Delay();
  int32_t        getRx1Delay();
  int32_t        getRx2Delay();
  LsmDataRate    getRx2DataRate();
  uint32_t       getRx2Frequency();
  LsmPingSlot    getPingSlot(); // Only for Class B
  uint8_t        getConfirmRetry();
  uint8_t        getUnconfirmedRetry();
  LsmNetworkType getNwkType();
  void           getChannelMask(char* outBuffer, size_t size);

  void         getDevID(char* outBuffer, size_t outSize);
  void         getDevPAC(char* outBuffer, size_t outSize);
  LsmRCChannel getRCChannel();
  uint8_t      getRadioPower();
  bool         getEncryptKey();
  bool         getEncryptPayload();
  int16_t      getLastRxRSSI();

  // Estado del Módulo
  bool isJoined() const;
  bool isTxBusy() const;

  // Sleep & Power Management
  bool sleep();
  bool wakeup();

private:
  LSM1x0A_AtParser  _parser;
  LsmUserCallback   _userCallback = nullptr;
  int16_t           _currentRSSI  = 0;
  SemaphoreHandle_t _stateMutex   = nullptr;

  // Reset pin
  int _resetPin;

  // Estado interno
  bool _isJoined;
  bool _isTxBusy;
  bool _isLSM110A;

  // Configuración de Recuperación
  LsmShadowConfig _shadowConfig;
  uint8_t         _maxTimeoutErrors = 3;
  uint8_t         _timeoutCounter   = 0;

  // Configuración de Logging
  LsmLogLevel _currentLogLevel = LsmLogLevel::INFO;

  // Timer de Seguridad para evitar bloqueos prolongados
  TimerHandle_t _safetyTimer;
  static void   safetyTimerCallback(TimerHandle_t xTimer);
  void          handleSafetyTimeout();
  void          startSafetyTimer(uint32_t durationMs);

  static void parserEventProxy(const char* type, const char* payload, void* ctx);
  void        handleInternalEvent(const char* type, const char* payload);

  // Helpers
  void     log(LsmLogLevel level, const char* format, ...);
  bool     canTransmit(bool requireLoRaMode);
  bool     restoreConfiguration();
  bool     readActualConfiguration();
  AtError  executeCommandWithRetries(const char* cmd, const char* expectedTag = nullptr, char* outBuffer = nullptr, size_t outSize = 0,
                                     uint32_t timeoutMs = 2000);
  uint32_t calculateBlockingTime(LsmDataRate dr, uint8_t totalTransmissions);
  void     setTxBusy(bool busy);

  bool handleSigfoxDownlinkResponse(AtError err, const char* rxBuffer);
  void parseTimestamp(const char* rxBuffer, char* outBuffer);
  void parseRSSI(const char* rxBuffer);
  bool isValidHex(const char* str, size_t maxLen = 256);
  bool isValidAscii(const char* str, size_t maxLen = 256);
};
#endif // LSM1X0A_CONTROLLER_H