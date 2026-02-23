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
  bool setChannelMask(const char* mask);
  bool setDevNonce(int nonce);
  bool resetDevNonce();

  // 3. Modos Operacionales (LoRaWAN)
  bool setJoinMode(LsmJoinMode mode);

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

private:
  LSM1x0A_Controller* _controller = nullptr;
  LsmJoinMode         _joinMode   = LsmJoinMode::OTAA;
};

#endif // LSM1X0A_LORAWAN_H
