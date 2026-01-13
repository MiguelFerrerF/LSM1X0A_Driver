#ifndef LSM1X0A_TYPES_H
#define LSM1X0A_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


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

// Estructura para almacenar datos de calidad de señal (Coverage Analysis)
struct LsmRxMetadata
{
  char slot[8]; // "1", "2", "C", "P_MC", etc.
  int  port;
  int  dataRate;
  int  rssi;
  int  snr;
  bool hasLinkCheck; // true si el servidor mandó info de gateways
  int  demodMargin;  // Solo válido si hasLinkCheck = true
  int  nbGateways;   // Solo válido si hasLinkCheck = true
};

// Tipos de Eventos Asíncronos que enviaremos al Callback
namespace LsmEvent
{
const char JOIN[]    = "JOIN";
const char TX[]      = "TX";
const char RX_DATA[] = "RX_DATA"; // El payload útil (Hex)
const char RX_META[] = "RX_META"; // Metadatos (RSSI, SNR, DR)
const char CLASS[]   = "CLASS";   // Cambio de Clase A/B/C
const char BEACON[]  = "BEACON";  // Info de Beacon
const char NVM[]     = "NVM";     // Guardado en Flash interna
const char INFO[]    = "INFO";    // Otros
} // namespace LsmEvent

#endif // LSM1X0A_TYPES_H