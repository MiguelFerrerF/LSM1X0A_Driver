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

  // Errores específicos de AT
  GENERIC_ERROR,       // AT_ERROR
  PARAM_ERROR,         // AT_PARAM_ERROR
  BUSY,                // AT_BUSY_ERROR - Importante: Reintentar
  TEST_PARAM_OVERFLOW, // "AT_TEST_PARAM_OVERFLOW"
  NO_NET_JOINED,       // AT_NO_NETWORK_JOINED - Importante: Hacer Join
  RX_ERROR,            // AT_RX_ERROR

  // Errores de Parser
  BOOT_ALERT, // "BOOTALERT" - Módulo reiniciado

  // Errores locales del Driver/Parser
  TIMEOUT,
  BUFFER_OVERFLOW,
  UNKNOWN
};

// Tipos de Eventos Asíncronos que enviaremos al Callback
namespace LsmEvent
{
const char JOIN[] = "JOIN";
const char TX[]   = "TX";   // Confirmación de envío
const char RX[]   = "RX";   // Dato recibido
const char INFO[] = "INFO"; // Mensajes de estado (LinkCheck, etc)
} // namespace LsmEvent

#endif // LSM1X0A_TYPES_H