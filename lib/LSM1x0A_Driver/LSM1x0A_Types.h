#ifndef LSM1X0A_TYPES_H
#define LSM1X0A_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

// Common baudrate for LSM1x0A modules
#define LSM1X0A_BAUDRATE 9600
// Common RX/TX pins for LSM1x0A modules
#define LSM1X0A_RX_PIN 14
#define LSM1X0A_TX_PIN 33
#define LSM1X0A_RESET_PIN 15
#define LSM1X0A_BOOT_ALERT_TIMEOUT_MS 5000

#define DEFAULT_MAX_RETRIES 3

// Buffer sizes for AT commands
#define AT_BUFFER_SIZE 256
#define AT_LORA_BUFFER_SIZE 512
#define AT_SIGFOX_BUFFER_SIZE 64

#define LSM1X0A_LORA_MODE 1
#define LSM1X0A_SIGFOX_MODE 0

#define LSM_INTERNAL_PROCESS_OVERHEAD 350

/**
 * @brief Represents the parsed AT command response codes and internal states of the LSM1x0A parser.
 * Maps directly to errors thrown by lora_at.c and lora_command.c inside the firmware.
 */
enum class AtError
{
  /** @brief Success token (e.g. \\r\\nOK\\r\\n) */
  OK = 0,

  // LoRaWAN Specific AT Errors
  /** @brief Generic LoRaWAN error (AT_ERROR) */
  GENERIC_ERROR,
  /** @brief Parameter error, out of bounds or malformed (AT_PARAM_ERROR) */
  PARAM_ERROR,
  /** @brief Module is busy. Important: Usually indicates a backoff/retry is needed (AT_BUSY_ERROR) */
  BUSY,
  /** @brief Parameter query overflow error (AT_TEST_PARAM_OVERFLOW) */
  TEST_PARAM_OVERFLOW,
  /** @brief Tried to send data without being joined to network. Important: Trigger Join (AT_NO_NETWORK_JOINED) */
  NO_NET_JOINED,
  /** @brief Serial or radio reception error (AT_RX_ERROR) */
  RX_ERROR,
  /** @brief Attempted Class B operation without it being enabled (AT_NO_CLASS_B_ENABLED) */
  NO_CLASS_B_ENABLE,
  /** @brief Transmission restricted due to ETSI Duty Cycle limits (AT_DUTYCYCLE_RESTRICTED) */
  DUTY_CYCLE_RESTRICT,
  /** @brief Cryptographic MAC error (AT_CRYPTO_ERROR) */
  CRYPTO_ERROR,

  // SigFox Specific Errors
  /** @brief Internal SigFox library error (AT_LIB_ERROR) */
  LIBRARY_ERROR,
  /** @brief Transmission timeout, channel busy or CS (AT_TX_TIMEOUT) */
  TX_TIMEOUT,
  /** @brief Reception window timeout (AT_RX_TIMEOUT) */
  RX_TIMEOUT,
  /** @brief Reconfiguration error (AT_RECONF_ERROR) */
  RECONF_ERROR,

  // Parser specific errors
  /** @brief Boot alert received spontaneously ("BOOTALERT"). Indicates module was restarted. */
  BOOT_ALERT,

  // Local Driver/Parser operational errors
  /** @brief The parser timed out waiting for the final \\r\\nOK\\r\\n token */
  TIMEOUT,
  /** @brief Internal string buffer overflowed */
  BUFFER_OVERFLOW,
  /** @brief Unknown AT string returned by the module */
  UNKNOWN
};

/**
 * @brief Structure to hold signal quality data (Coverage Analysis) and payload metadata.
 */
struct LsmRxMetadata
{
  /** @brief Rx Slot ("1", "2", "C", "P_MC") */
  char slot[8] = "";
  /** @brief Downlink port */
  int port = 0;
  /** @brief Operational Data Rate */
  int dataRate = 0;
  /** @brief Received Signal Strength Indicator (dBm) */
  int rssi = 0;
  /** @brief Signal-to-Noise Ratio (dB) */
  int snr = 0;
  /** @brief Flag indicating if the server replied with LinkCheck info */
  bool hasLinkCheck = false;
  /** @brief Demodulation Margin (only valid if hasLinkCheck = true) */
  int demodMargin = 0;
  /** @brief Number of Gateways that received the uplink (only valid if hasLinkCheck = true) */
  int nbGateways = 0;
};

/**
 * @brief Standard logging hierarchy levels for Driver Verbosity.
 */
enum class LsmLogLevel
{
  /** @brief Absolute silence, no logs emitted */
  SILENCE = 0,
  /** @brief Only critical hardware failures or unrecoverable timeouts */
  ERROR,
  /** @brief Warnings about non-optimal operation or retries */
  WARN,
  /** @brief High-level application state (Join Success, Packet Sent) */
  INFO,
  /** @brief Complete AT command traffic and flow tracing (TX: AT+... / RX: OK) */
  DEBUG,
  /** @brief Absolutely everything, including internal module raw dumps if available */
  VERBOSE
};

/** @brief Represents the physical device model detected */
enum class LsmModuleType
{
  UNKNOWN = 0,
  LSM100A,
  LSM110A
};

/** @brief The active network protocol stack of the dual-mode radio */
enum class LsmMode
{
  SIGFOX = 0,
  LORAWAN,
  MODE_UNKNOWN
};

/** @brief LoRaWAN Join strategy */
enum class LsmJoinMode
{
  ABP = 0,
  OTAA,
  JOIN_MODE_UNKNOWN
};

/** @brief LoRaWAN Device Class */
enum class LsmClass
{
  CLASS_A = 0,
  CLASS_B,
  CLASS_C,
  CLASS_UNKNOWN
};

/** @brief LoRaWAN Network Type (Public/Private Sync Word) */
enum class LsmNetworkType
{
  PUBLIC = 0,
  PRIVATE,
  NETWORK_TYPE_UNKNOWN
};

/** @brief LoRaWAN Data Rates (DR0-DR7) */
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

/** @brief Transmit Power levels normalized against regional max output */
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
/** @brief LoRaWAN Regional Band IDs */
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

/**
 * @brief Constants for multi-channel subband selection (Channel Mask).
 * Bitwise OR can be used to activate multiple banks: (SUB_BAND_1 | SUB_BAND_2)
 */
enum LsmSubBand
{
  SUB_BAND_NONE = 0x0000,
  SUB_BAND_1    = 0x0001,
  SUB_BAND_2    = 0x0002,
  SUB_BAND_3    = 0x0004,
  SUB_BAND_4    = 0x0008,
  SUB_BAND_5    = 0x0010,
  SUB_BAND_6    = 0x0020,
  SUB_BAND_7    = 0x0040,
  SUB_BAND_8    = 0x0080,
  SUB_BAND_9    = 0x0100, /**< CN470 specific */
  SUB_BAND_10   = 0x0200, /**< CN470 specific */
  SUB_BAND_11   = 0x0400, /**< CN470 specific */
  SUB_BAND_12   = 0x0800, /**< CN470 specific */
  SUB_BAND_ALL  = 0xFFFF
};

/** @brief Periodicity for Class B Ping Slots */
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

/** @brief Sigfox Regional Configurations (Macro-Channels) */
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

/** @brief Payload encoding format for Sigfox transmissions */
enum class LsmSigfoxDataType
{
  BIT = 0,
  ASCII_DATA,
  HEX_DATA,
  OOB
};

/**
 * @brief Asynchronous Event Types dispatched to the User Callback.
 * @cond
 */
namespace LsmEvent
{
const char JOIN[]       = "JOIN";
const char TX[]         = "TX";
const char RX_DATA[]    = "RX_DATA";    /**< Useful hexadecimal payload */
const char RX_META[]    = "RX_META";    /**< Window Quality Metadata (RSSI, SNR, DR) */
const char RX_TIMEOUT[] = "RX_TIMEOUT"; /**< Explicit reception window timeout */
const char CLASS[]      = "CLASS";      /**< Device Class switch */
const char BEACON[]     = "BEACON";     /**< Beacon Information */
const char NVM[]        = "NVM";        /**< Saved context to internal flash */
const char CHMASK[]     = "CHMASK";     /**< Channel mask setting event */
} // namespace LsmEvent
/** @endcond */

/**
 * @brief Common event outcome status constants.
 * @cond
 */
namespace LsmStatus
{
const char SUCCESS[] = "SUCCESS";
const char FAILED[]  = "FAILED";
} // namespace LsmStatus
/** @endcond */

/**
 * @brief Raw AT commands and strings used by the module's protocol.
 * @cond
 */
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

// Mac Network Configuration
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
const char DEV_ID[]             = "AT$ID=";
const char DEV_PAC[]            = "AT$PAC=";
const char RC_CHANNEL[]         = "AT$RC=";
const char SEND_BIT[]           = "AT$SB=";
const char SEND_BIT_CONFIRMED[] = "AT$SB=1,1,2";
const char SEND_PAYLOAD[]       = "AT$SF=";
const char SEND_STRING[]        = "AT$SH=";
const char SEND_OOB[]           = "ATS300";
const char RADIO_POWER[]        = "ATS302=";
const char ENCRYPT_KEY[]        = "ATS410=";
const char ENCRYPT_PAYLOAD[]    = "ATS411=";

// SigFox RF Test & Scan Commands
const char TEST_CW[]      = "AT$CW=";
const char TEST_PRBS9[]   = "AT$PN=";
const char TEST_MONARCH[] = "AT$MN=";
const char TEST_MODE[]    = "AT$TM=";
const char TEST_RL[]      = "AT$RL";
const char TEST_SL[]      = "AT$SL";
const char TEST_SP2P[]    = "AT$SP2P";
const char TEST_RP2P[]    = "AT$RP2P";
} // namespace LsmAtCommand
/** @endcond */

#endif // LSM1X0A_TYPES_H
