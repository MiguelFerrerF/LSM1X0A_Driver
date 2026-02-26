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
#define LSM1X0A_RESET_PIN 15
#define LSM1X0A_BOOT_ALERT_TIMEOUT_MS 5000

#define DEFAULT_MAX_RETRIES 3

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
  char slot[8]      = ""; // "1", "2", "C", "P_MC", etc.
  int  port         = 0;
  int  dataRate     = 0;
  int  rssi         = 0;
  int  snr          = 0;
  bool hasLinkCheck = false; // true si el servidor mandó info de gateways
  int  demodMargin  = 0;     // Solo válido si hasLinkCheck = true
  int  nbGateways   = 0;     // Solo válido si hasLinkCheck = true
};

// Niveles de Log (Jerarquía estándar)
enum class LsmLogLevel
{
  SILENCE = 0, // Silencio total
  ERROR,       // Solo fallos críticos (Hardware, Timeouts)
  WARN,        // Advertencias (Operación no óptima)
  INFO,        // Estado de alto nivel (Join Success, Packet Sent)
  LSM_DEBUG,   // Tráfico de comandos AT (TX: AT+... / RX: OK)
  VERBOSE      // Todo (incluyendo dumps internos si los hubiera)
};

enum class LsmModuleType
{
  UNKNOWN = 0,
  LSM100A,
  LSM110A
};

enum class LsmMode
{
  SIGFOX = 0,
  LORAWAN,
  MODE_UNKNOWN
};
enum class LsmJoinMode
{
  ABP = 0,
  OTAA,
  JOIN_MODE_UNKNOWN
};
enum class LsmClass
{
  CLASS_A = 0,
  CLASS_B,
  CLASS_C,
  CLASS_UNKNOWN
};
enum class LsmNetworkType
{
  PUBLIC = 0,
  PRIVATE,
  NETWORK_TYPE_UNKNOWN
};
enum class LsmDataRate
{
  DR_0 = 0,
  DR_1,
  DR_2,
  DR_3,
  DR_4,
  DR_5,
  DR_6,
  DR_7,
  DR_UNKNOWN
};
enum class LsmTxPower
{
  TP_MAX = 0,
  TP_MAX_MINUS_2,
  TP_MAX_MINUS_4,
  TP_MAX_MINUS_6,
  TP_MAX_MINUS_8,
  TP_MAX_MINUS_10,
  TP_MAX_MINUS_12,
  TP_MAX_MINUS_14,
  TP_UNKNOWN
};
enum class LsmBand
{
  AS923_1 = 0,
  AU915,
  CN470,
  CN779,
  EU433,
  EU868,
  KR920,
  IN865,
  US915,
  RU864,
  AS923_4,
  BAND_UNKNOWN
};
enum class LsmPingSlot
{
  EVERY_1_SEC = 0,
  EVERY_2_SEC,
  EVERY_4_SEC,
  EVERY_8_SEC,
  EVERY_16_SEC,
  EVERY_32_SEC,
  EVERY_64_SEC,
  EVERY_128_SEC,
  PING_SLOT_UNKNOWN
};
enum class LsmRCChannel
{
  RC3A = 0,
  RC1,
  RC2,
  RC3C,
  RC4,
  RC5,
  RC6,
  RC7,
  RC_UNKNOWN
};
enum class LsmSigfoxDataType
{
  BIT = 0,
  ASCII_DATA,
  HEX_DATA,
  OOB
};

// Tipos de Eventos Asíncronos que enviaremos al Callback
namespace LsmEvent
{
const char JOIN[]       = "JOIN";
const char TX[]         = "TX";
const char RX_DATA[]    = "RX_DATA";    // El payload útil (Hex)
const char RX_META[]    = "RX_META";    // Metadatos (RSSI, SNR, DR)
const char RX_TIMEOUT[] = "RX_TIMEOUT"; // Timeout explícito de la ventana de recepción
const char CLASS[]      = "CLASS";      // Cambio de Clase A/B/C
const char BEACON[]     = "BEACON";     // Info de Beacon
const char NVM[]        = "NVM";        // Guardado en Flash interna
const char INFO[]       = "INFO";       // Otros
const char LOG[]        = "LOG";        // Mensajes de Log
const char VERBOSE[]    = "VERBOSE";
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
const char SSW_VERSION[]   = "AT$SSWVER=?";
const char LOCAL_TIME[]    = "AT+LTIME=?";

// LoRaWAN Specific Commands
const char APP_EUI[]   = "AT+APPEUI=";
const char DEV_EUI[]   = "AT+DEUI=";
const char APP_KEY[]   = "AT+APPKEY=";
const char NWK_KEY[]   = "AT+NWKKEY=";
const char DEV_ADDR[]  = "AT+DADDR=";
const char APP_SKEY[]  = "AT+APPSKEY=";
const char NWK_SKEY[]  = "AT+NWKSKEY=";
const char NWK_ID[]    = "AT+NWKID=";
const char DEVNONCE[]  = "AT+DEVNONCE=";
const char FRAME_CNT[] = "AT+ABPFCNT=";

// Operaciones de Red y Configuración MAC
const char ADAPTIVE_DR[]     = "AT+ADR=";
const char DR[]              = "AT+DR=";
const char TX_POWER[]        = "AT+TXP=";
const char BAND[]            = "AT+BAND=";
const char CLASS[]           = "AT+CLASS=";
const char DUTY_CYCLE[]      = "AT+DCS=";
const char JOIN_DELAY_1[]    = "AT+JN1DL=";
const char JOIN_DELAY_2[]    = "AT+JN2DL=";
const char RX1_DELAY[]       = "AT+RX1DL=";
const char RX2_DELAY[]       = "AT+RX2DL=";
const char RX2_DR[]          = "AT+RX2DR=";
const char RX2_FREQ[]        = "AT+RX2FQ=";
const char PING_SLOT[]       = "AT+PGSLOT=";
const char LINK_CHECK[]      = "AT+LINKC";
const char CONFIRM_RETRY[]   = "AT+CNFRETX=";
const char UNCONFIRM_RETRY[] = "AT+UNCNFRETX=";
const char NETWORK_TYPE[]    = "AT+NWKTYPE=";
const char CHANNEL_MASK[]    = "AT+CHMASK=";
const char JOIN[]            = "AT+JOIN=";
const char SEND[]            = "AT+SEND=";

// LoRaWAN RF Test & Certification Commands
const char TTEST_TONE[]   = "AT+TTONE";
const char TTEST_RSSI[]   = "AT+TRSSI";
const char TTEST_CONF[]   = "AT+TCONF=";
const char TTEST_TX[]     = "AT+TTX=";
const char TTEST_RX[]     = "AT+TRX=";
const char TTEST_TX_HOP[] = "AT+TTH=";
const char TTEST_STOP[]   = "AT+TOFF";
const char CERTIF_MODE[]  = "AT+CERTIF=";
const char TTEST_MTX[]    = "AT+MTX";
const char TTEST_MRX[]    = "AT+MRX";
const char CERTIF_SEND[]  = "AT+CERTISEND";
const char P2P_CONF[]     = "AT+PCONF=";
const char P2P_SEND[]     = "AT+PSEND=";
const char P2P_RECV[]     = "AT+PRECV=";

// SigFox Specific Commands
const char DEV_ID[]             = "AT$ID";
const char DEV_PAC[]            = "AT$PAC=";
const char RC_CHANNEL[]         = "AT$RC=";
const char SEND_BIT[]           = "AT$SB=";
const char SEND_BIT_CONFIRMED[] = "AT$SB=1,1,2";
const char SEND_FRAME[]         = "AT$SF=";
const char SEND_HEX[]           = "AT$SH=";
const char SEND_OOB[]           = "ATS300";
const char RADIO_POWER[]        = "ATS302=";
const char ENCRYPT_KEY[]        = "ATS410=";
const char ENCRYPT_PAYLOAD[]    = "ATS411=";
} // namespace LsmAtCommand

#endif // LSM1X0A_TYPES_H
