#ifndef LSM1X0A_TYPES_H
#define LSM1X0A_TYPES_H

// Baudrate común para módulos LSM1x0A
#define LSM1X0A_BAUDRATE 9600
// RX/TX pins comunes para módulos LSM1x0A
#define LSM1X0A_RX_PIN 14
#define LSM1X0A_TX_PIN 33

// Dimensión segura basada en Payload LoRaWAN máximo en Hex + Protocolo
#define AT_BUFFER_SIZE 512

// Mapeo directo de lora_at.h y lora_command.c
enum class AtError
{
  OK = 0,
  GENERIC_ERROR,       // ERROR
  PARAM_ERROR,         // AT_PARAM_ERROR
  BUSY,                // AT_BUSY_ERROR
  TEST_PARAM_OVERFLOW, // AT_TEST_PARAM_OVERFLOW
  NO_NET_JOINED,       // AT_NO_NETWORK_JOINED
  RX_ERROR,            // AT_RX_ERROR
  TIMEOUT,             // NO MODUKE RESPONSE
  BUFFER_OVERFLOW,     // BUFFER_OVERFLOW
  BOOT_ALERT,          // BOOT ALERT
  UNKNOWN
};

#endif // LSM1X0A_TYPES_H