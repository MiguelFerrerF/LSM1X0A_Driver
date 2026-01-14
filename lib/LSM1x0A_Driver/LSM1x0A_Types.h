#ifndef LSM1X0A_TYPES_H
#define LSM1X0A_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

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

  // Errores específicos de AT LoRaWAN
  GENERIC_ERROR,       // AT_ERROR
  PARAM_ERROR,         // AT_PARAM_ERROR
  BUSY,                // AT_BUSY_ERROR - Importante: Reintentar
  TEST_PARAM_OVERFLOW, // AT_TEST_PARAM_OVERFLOW - Error en consulta de parámetros
  NO_NET_JOINED,       // AT_NO_NETWORK_JOINED - Importante: Hacer Join
  RX_ERROR,            // AT_RX_ERROR
  NO_CLASS_B_ENABLE,   // AT_NO_CLASS_B_ENABLED
  DUTY_CYCLE_RESTRICT, // AT_DUTYCYCLE_RESTRICTED
  CRYPTO_ERROR,        // AT_CRYPTO_ERROR

  // Errores específicos de SigFox
  LIBRARY_ERROR, // AT_LIB_ERROR - Error interno de la librería SigFox
  TX_TIMEOUT,    // AT_TX_TIMEOUT - Timeout en transmisión
  RX_TIMEOUT,    // AT_RX_TIMEOUT - Timeout en recepción
  RECONF_ERROR,  // AT_RECONF_ERROR - Error de reconfiguración

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

// Niveles de Log (Jerarquía estándar)
enum class LsmLogLevel
{
  NONE = 0, // Silencio total
  ERROR,    // Solo fallos críticos (Hardware, Timeouts)
  INFO,     // Estado de alto nivel (Join Success, Packet Sent)
  DEBUG,    // Tráfico de comandos AT (TX: AT+... / RX: OK)
  VERBOSE   // Todo (incluyendo dumps internos si los hubiera)
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
const char LOG[]     = "LOG";     // Mensajes de Log
const char VERBOSE[] = "VERBOSE";
} // namespace LsmEvent

// Commandos AT especiales para el módulo LSM1x0A
namespace LsmAtCommand
{
// General AT Commands
const char MODE[]          = "AT+MODE=";
const char RESET[]         = "ATZ";
const char BAUDRATE[]      = "AT+BAUDRATE=";
const char FACTORY_RESET[] = "AT+RFS";
const char VERBOSE_LEVEL[] = "AT+VL=";
const char BATTERY[]       = "AT+BAT=?";
const char FW_VERSION[]    = "AT+VER=?";
const char LOCAL_TIME[]    = "AT+LTIME=?";

// LoRaWAN Specific Commands
const char APP_EUI[]   = "AT+APPEUI=";   // AppEUI o JoinEUI del dispositivo. Se guarda en NVM
const char DEV_EUI[]   = "AT+DEUI=";     // Identificador único del dispositivo. No se guarda en NVM
const char APP_KEY[]   = "AT+APPKEY=";   // AppKey para OTAA. Se guarda en NVM
const char NWK_KEY[]   = "AT+NWKKEY=";   // NwkKey para ABP. Se guarda en NVM
const char DEV_ADDR[]  = "AT+DADDR=";    // Dirección del dispositivo para ABP. No se guarda en NVM
const char APP_SKEY[]  = "AT+APPSKEY=";  // AppSKey para OTAA V1.1. Se guarda en NVM
const char NWK_SKEY[]  = "AT+NWKSKEY=";  // NwkSKey para OTAA V1.1. Se guarda en NVM
const char NWK_ID[]    = "AT+NWKID=";    // Network ID. No se guarda en NVM
const char DEVNONCE[]  = "AT+DEVNONCE="; // DevNonce para OTAA. Se guarda en NVM
const char FRAME_CNT[] = "AT+ABPFCNT=";  // Frame Counter para ABP. Se guarda en NVM

// Operaciones de Red y Configuración MAC
const char ADAPTIVE_DR[]     = "AT+ADR=";       // Adaptive Data Rate. No se guarda en NVM
const char DR[]              = "AT+DR=";        // Data Rate fijo. No se guarda en NVM
const char TX_POWER[]        = "AT+TXP=";       // Potencia de transmisión. No se guarda en NVM
const char BAND[]            = "AT+BAND=";      // Banda de frecuencia. No se guarda en NVM
const char CLASS[]           = "AT+CLASS=";     // Clase LoRaWAN (A, B, C). Se guarda en NVM
const char DUTY_CYCLE[]      = "AT+DCS=";       // Duty Cycle. No se guarda en NVM
const char JOIN_DELAY_1[]    = "AT+JN1DL=";     // Retardo de Join Window 1. No se guarda en NVM
const char JOIN_DELAY_2[]    = "AT+JN2DL=";     // Retardo de Join Window 2. No se guarda en NVM
const char RX1_DELAY[]       = "AT+RX1DL=";     // Retardo de RX1 (Clase A). No se guarda en NVM
const char RX2_DELAY[]       = "AT+RX2DL=";     // Retardo de RX2 (Clase A). No se guarda en NVM
const char RX2_DR[]          = "AT+RX2DR=";     // Data Rate de RX2. No se guarda en NVM
const char RX2_FREQ[]        = "AT+RX2FQ=";     // Frecuencia de RX2. No se guarda en NVM
const char PING_SLOT[]       = "AT+PGSLOT=";    // Intervalo de Ping Slot (Clase B). No se guarda en NVM
const char LINK_CHECK[]      = "AT+LINKC";      // Link Check en la siguiente transmisión
const char CONFIRM_RETRY[]   = "AT+CNFRETX=";   // Número de reintentos para confirmados. Se guarda en NVM
const char UNCONFIRM_RETRY[] = "AT+UNCNFRETX="; // Número de reintentos para no confirmados. Se guarda en NVM
const char NETWORK_TYPE[]    = "AT+NWKTYPE=";   // Tipo de red (Pública, Private). Se guarda en NVM
const char CHANNEL_MASK[]    = "AT+CHMASK=";    // Máscara de canales.
const char JOIN[]            = "AT+JOIN=";      // Iniciar proceso de Join (OTAA o ABP)
const char SEND[]            = "AT+SEND=";      // Enviar datos (Confirmed/Unconfirmed)

// SigFox Specific Commands
const char DEV_ID[]             = "AT$ID";       // Identificador único del dispositivo SigFox
const char PAC[]                = "AT$PAC=";     // PAC del dispositivo SigFox
const char RC_CHANNEL[]         = "AT$RC=";      // Canal de Radio SigFox
const char SEND_BITS[]          = "AT$SB=";      // Enviar datos en modo bits
const char SEND_BIT_CONFIRMED[] = "AT$SB=1,1,2"; // Bit=1, Downlink=1, TxRepeat=2 (Join Simulado)
const char SEND_FRAME[]         = "AT$SF=";      // Enviar datos en modo frame (ASCII)
const char SEND_HEX[]           = "AT$SH=";      // Enviar datos en modo hexadecimal
const char SEND_OOB[]           = "AT$S300";     // Enviar datos en modo Out-Of-Band (reporte gratuito)

} // namespace LsmAtCommand

#endif // LSM1X0A_TYPES_H