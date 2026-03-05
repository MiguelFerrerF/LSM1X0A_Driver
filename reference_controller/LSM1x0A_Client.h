#ifndef LSM1X0A_CLIENT_H
#define LSM1X0A_CLIENT_H

#include "LSM1x0A_Controller.h"
#include "UartDriver.h"

#define LSM1X0A_RESET_PIN 15

#define NOT_JOINED 0
#define JOINED 1
#define JOIN_IN_PROCESS 2

struct LoRaData
{
  uint8_t length;
  uint8_t values[80];

  // Function to obtain hex representation of the array values
  void toHexString(char* hexString, size_t hexStringSize) const
  {
    size_t index = 0;
    for (uint8_t i = 0; i < length && index < hexStringSize - 1; ++i) {
      int written = snprintf(hexString + index, hexStringSize - index, "%02X", values[i]);
      if (written > 0) {
        index += written;
      }
      else {
        break; // Stop if there's an error in writing
      }
    }
    hexString[index] = '\0'; // Null-terminate the string
  }
  // Asign data to the struct values
  void setValues(const uint8_t* inputValues, uint8_t inputLength)
  {
    length = inputLength > sizeof(values) ? sizeof(values) : inputLength;
    memcpy(values, inputValues, length);
  }
};

enum class ClientState : uint8_t
{
  IDLE = 0,         /*!< Client initialized but not active */
  JOINING,          /*!< Attempting network join */
  NETWORK_READY,    /*!< Successfully joined, ready to transmit */
  TRANSMITTING,     /*!< Data transmission in progress */
  WAITING_DOWNLINK, /*!< Awaiting downlink in RX window */
  ERROR_RECOVERY,   /*!< Recovering from persistent failure */
  SLEEPING,         /*!< Low-power mode */
  NEED_RETRY,       /*!< Last transmission failed, retry needed */
  ERROR             /*!< Unrecoverable error state */
};

typedef void (*LoRaDataCallback)(const uint8_t* data, size_t length);
typedef void (*LsmUserCallback)(const char* type, const char* payload);
typedef void (*LoRaDebugCallback)(const char* message);

class LSM1x0A_Client
{
public:
  LSM1x0A_Client();
  ~LSM1x0A_Client();

  void begin();
  void begin(LoRaDataCallback downlinkCallback, LoRaDebugCallback userCallback);
  void defaultConfig(uint8_t band, uint8_t power = 0, bool adr = true, uint8_t dr = 2, bool ack = true);
  void joinNetwork(LsmJoinMode mode = LsmJoinMode::OTAA);
  void sendData(LoRaData data);
  bool recoverModule();

  //----------------------- Getters y setters -----------------------//
  // Setters
  void setDataCallback(LoRaDataCallback dataCallback);
  void setAppEUI(const unsigned char appEUIBytes[8]);
  void setAppKey(const unsigned char appKeyBytes[16]);
  void setADR(const bool adr);
  void setDR(const uint8_t dr);
  void setClass(const char LoRaClass);
  void setPower(const uint8_t power);
  void setDevAddr(const unsigned char devAddrBytes[4]);
  void setNetID(const uint8_t id);
  void setBand(uint8_t band);
  bool setDevNonce(uint16_t nonce);
  void setPort(const uint8_t port);
  void setAck(const bool ack);
  void setRetransmission(const uint8_t retransmissions);
  void setChannelMask(uint8_t channel);
  void setDutyCycle(const bool dutyCycle);
  void setLinkCheck();

  // Getters
  bool        getModuleType();
  ClientState getState();

  void    getModuleVersion(char* outBuffer, size_t outSize);
  void    getDevEUI(unsigned char devEUIBytes[8]);
  void    getAppEUI(unsigned char appEUIBytes[8]);
  void    getAppKey(unsigned char appKeyBytes[16]);
  bool    getADR();
  uint8_t getDR();
  char    getClass();
  uint8_t getPower();
  void    getDevAddr(unsigned char devAddrBytes[4]);
  uint8_t getNetID();
  uint8_t getPort();
  // uint8_t getJoinMode();
  uint8_t getJoinStatus();
  uint8_t getRetransmission();
  uint8_t getBand();
  bool    getAck();
  uint8_t getChannelMask();
  bool    getDutyCycle();

private:
  LSM1x0A_Controller* controller;
  LoRaDataCallback    dataCallback;
  LoRaDebugCallback   debugCallback;
  UartDriver*         uartDriver;
  ClientState         state;

  uint8_t  lwPort        = 33;
  bool     isAck         = false;
  uint8_t  statusJoin    = NOT_JOINED;
  uint8_t  sendFailCount = 0;
  LoRaData lastSentData; // Guardamos el último mensaje enviado para posibles reintentos

  static LSM1x0A_Client* instance;
  static void            onModemEvent(const char* type, const char* payload);

  // Internal helper methods
  void handleDownlink(const char* payload);
  void updateState(ClientState newState);
  // void logEvent(const char* event);
  // void resetModule();
  void logDebug(const char* message);
};

#endif // LSM1X0A_CLIENT_H