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

// Tamaños de buffer para comandos AT
#define AT_BUFFER_SIZE 256
#define AT_LORA_BUFFER_SIZE 512
#define AT_SIGFOX_BUFFER_SIZE 64

#define LSM1X0A_LORA_MODE 1
#define LSM1X0A_SIGFOX_MODE 0

#define LSM_INTERNAL_PROCESS_OVERHEAD 350

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
  SILENCE = 0, // Silencio total
  ERROR,       // Solo fallos críticos (Hardware, Timeouts)
  WARN,        // Advertencias (Operación no óptima)
  INFO,        // Estado de alto nivel (Join Success, Packet Sent)
  LSM_DEBUG,       // Tráfico de comandos AT (TX: AT+... / RX: OK)
  VERBOSE      // Todo (incluyendo dumps internos si los hubiera)
};

enum class LsmJoinMode
{
  OTAA = 0, // Over-The-Air Activation (LoRaWAN)
  ABP,      // Activation By Personalization (LoRaWAN)
};

enum class LsmClass
{
  CLASS_A = 0, // Clase A
  CLASS_B,     // Clase B
  CLASS_C      // Clase C
};             // LoRaWAN Classes

enum class LsmNetworkType
{
  PUBLIC = 0, // Red pública LoRaWAN (TTN, etc.)
  PRIVATE     // Red privada LoRaWAN
};            // LoRaWAN Network Types

enum class LsmDataRate
{
  DR_0 = 0, // SF12 - 250bps
  DR_1,     // SF11 - 440bps
  DR_2,     // SF10 - 980bps
  DR_3,     // SF9  - 1760bps
  DR_4,     // SF8  - 3125bps
  DR_5,     // SF7  - 5470bps
  DR_6,     // SF7B - 11000bps
  DR_7,     // FSK 50kbps
};          // LoRaWAN Data Rates

enum class LsmTxPower
{
  TP_MAX = 0,      // Max Power
  TP_MAX_MINUS_2,  // Max Power - 2 dB
  TP_MAX_MINUS_4,  // Max Power - 4 dB
  TP_MAX_MINUS_6,  // Max Power - 6 dB
  TP_MAX_MINUS_8,  // Max Power - 8 dB
  TP_MAX_MINUS_10, // Max Power - 10 dB
  TP_MAX_MINUS_12, // Max Power - 12 dB
  TP_MAX_MINUS_14  // Max Power - 14 dB
};                 // LoRaWAN Transmit Power Levels

enum class LsmBand
{
  AS923_1 = 0, // AS923 original
  AU915,
  CN470,
  CN779,
  EU433,
  EU868,
  KR920,
  IN865,
  US915,
  RU864,
  AS923_4 // AS923-1 Japón
};        // LoRaWAN Bands

enum class LsmPingSlot
{
  EVERY_1_SEC = 0,
  EVERY_2_SEC,
  EVERY_4_SEC,
  EVERY_8_SEC,
  EVERY_16_SEC,
  EVERY_32_SEC,
  EVERY_64_SEC,
  EVERY_128_SEC
}; // LoRaWAN Ping Slot periodicity for Class B

enum class LsmRCChannel
{
  RC3A = 0,
  RC1,
  RC2,
  RC3C,
  RC4,
  RC5,
  RC6,
  RC7
}; // SigFox Regional Channels

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

// Estados comunes de respuesta
namespace LsmStatus
{
const char SUCCESS[] = "SUCCESS";
const char FAILED[]  = "FAILED";
} // namespace LsmStatus

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
const char DEV_PAC[]            = "AT$PAC=";     // PAC del dispositivo SigFox
const char RC_CHANNEL[]         = "AT$RC=";      // Canal de Radio SigFox
const char SEND_BIT[]           = "AT$SB=";      // Enviar datos en modo bit (0 o 1)
const char SEND_BIT_CONFIRMED[] = "AT$SB=1,1,2"; // Bit=1, Downlink=1, TxRepeat=2 (Join Simulado)
const char SEND_FRAME[]         = "AT$SF=";      // Enviar datos en modo frame (ASCII)
const char SEND_HEX[]           = "AT$SH=";      // Enviar datos en modo hexadecimal
const char SEND_OOB[]           = "ATS300";      // Enviar datos en modo Out-Of-Band (reporte gratuito)
const char RADIO_POWER[]        = "ATS302=";     // Potencia de transmisión SigFox
const char ENCRYPT_KEY[]        = "ATS410=";     // Clave de encriptación SigFox (private or public)
const char ENCRYPT_PAYLOAD[]    = "ATS411=";     // Habilitar/deshabilitar encriptación de payload

} // namespace LsmAtCommand

enum class LsmSigfoxDataType
{
  BIT = 0,
  ASCII_DATA,
  HEX_DATA,
  OOB
};

// Máscaras de 8 canales (EU868, etc.)
static const char* M_8CH[] = {"000F", "00F0", "00FF"};

// Máscaras de 72 canales (US915, AU915) - Usan 5 bloques de 16 bits (80 bits totales)
static const char* M_72CH[] = {
  "00FF:0000:0000:0000:0000", "FF00:0000:0000:0000:0000", "0000:00FF:0000:0000:0000", "0000:FF00:0000:0000:0000",
  "0000:0000:00FF:0000:0000", "0000:0000:FF00:0000:0000", "0000:0000:0000:00FF:0000", "0000:0000:0000:FF00:0000",
  "0000:0000:0000:0000:00FF", "FFFF:FFFF:FFFF:FFFF:00FF" // Modo ALL (255)
};

// Máscaras de 96 canales (CN470) - Usan 6 bloques de 16 bits
static const char* M_96CH[] = {
  "00FF:0000:0000:0000:0000:0000", "FF00:0000:0000:0000:0000:0000", "0000:00FF:0000:0000:0000:0000", "0000:FF00:0000:0000:0000:0000",
  "0000:0000:00FF:0000:0000:0000", "0000:0000:FF00:0000:0000:0000", "0000:0000:0000:00FF:0000:0000", "0000:0000:0000:FF00:0000:0000",
  "0000:0000:0000:0000:00FF:0000", "0000:0000:0000:0000:FF00:0000", "0000:0000:0000:0000:0000:00FF", "0000:0000:0000:0000:0000:FF00",
  "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF" // Modo ALL (255)
};

struct LsmMaskConfig
{
  LsmBand      band;
  const char** masks;
  uint8_t      count; // Cantidad de sub-bandas (sin contar el índice ALL)
};

static const LsmMaskConfig BAND_MAPS[] = {{LsmBand::EU868, M_8CH, 2},  {LsmBand::AS923_1, M_8CH, 2}, {LsmBand::KR920, M_8CH, 2},
                                          {LsmBand::IN865, M_8CH, 2},  {LsmBand::RU864, M_8CH, 2},   {LsmBand::US915, M_72CH, 9},
                                          {LsmBand::AU915, M_72CH, 9}, {LsmBand::CN470, M_96CH, 12}};

static const uint16_t LSM_TOA_MS[] = {
  1304, // DR0 (SF12)
  808,  // DR1 (SF11)
  439,  // DR2 (SF10)
  315,  // DR3 (SF9)
  232,  // DR4 (SF8)
  197   // DR5 (SF7)
};

// Estructura para mantener el estado deseado (Shadow Configuration)
struct LsmShadowConfig
{
  bool isLoRaMode;

  // Non Persistent Settings

  char        devEui[24]         = {0};
  char        devAddr[12]        = {0};
  char        nwkID[12]          = {0};
  bool        adrEnabled         = true;
  LsmDataRate dr                 = LsmDataRate::DR_5;
  LsmTxPower  txPower            = LsmTxPower::TP_MAX;
  LsmBand     band               = LsmBand::EU868;
  uint8_t     subBand            = 0;
  bool        dutyCycle          = false;
  int32_t     join1Delay         = 5000;
  int32_t     join2Delay         = 6000;
  int32_t     rx1Delay           = 1000;
  int32_t     rx2Delay           = 2000;
  LsmDataRate rx2DataRate        = LsmDataRate::DR_0;
  uint32_t    rx2Frequency       = 0;
  int         channelMaskSubBand = 0;
  uint8_t     confirmRetry       = 1;
  uint8_t     unconfirmRetry     = 1;

  LsmRCChannel rcChannel      = LsmRCChannel::RC1;
  uint8_t      radioPower     = 20;
  bool         encryptKey     = false;
  bool         encryptPayload = false;
};

#endif // LSM1X0A_TYPES_H
