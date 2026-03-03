#ifndef LSM1X0A_AT_PARSER_H
#define LSM1X0A_AT_PARSER_H

#include "LSM1x0A_Types.h"
#include "UartDriver.h"
#include "freertos/semphr.h"
#include <cstdio>
#include <cstring>

// Callback for asynchronous events (URC) from the module. Examples:
// type: "RX", "JOIN", etc.
// payload: The rest of the line
// ctx: User context
typedef void (*AtEventCallback)(const char* type, const char* payload, void* ctx);

/**
 * @brief Thread-safe AT Command Parser for the LSM100A/LSM110A modules.
 * Handles UART communication, timeouts, async events (URC), and parsing responses.
 */
class LSM1x0A_AtParser
{
public:
  LSM1x0A_AtParser();
  ~LSM1x0A_AtParser();

  /**
   * @brief Initializes the AT Parser with a UART driver and an optional asynchronous event callback.
   * @param driver Pointer to the UartDriver instance.
   * @param onEvent Callback for asynchronous URC (Unsolicited Result Code) events.
   * @param eventCtx Context pointer passed back in the callback.
   * @return true if successfully initialized.
   */
  bool init(UartDriver* driver, AtEventCallback onEvent = nullptr,
            void* eventCtx = nullptr); /**
                                        * @brief Performs a "Ping" activation by sending AT commands until OK is received.
                                        * Handles the module's low-power state.
                                        * @return true if the module woke up and responded correctly.
                                        */
  bool wakeUp(uint8_t retries = DEFAULT_MAX_RETRIES, uint32_t delayMs = 500);

  /**
   * @brief Sends a simple command and waits for OK/ERROR.
   * Example: sendCommand("AT+JOIN");
   */
  AtError sendCommand(const char* cmd, uint32_t timeoutMs = 2000);

  /**
   * @brief Waits for an event (like BOOTALERT) without sending any command.
   * Useful for hardware reset where the module speaks on its own.
   * @return The AtError corresponding to the event (e.g., AtError::BOOT_ALERT).
   */
  AtError waitForEvent(uint32_t timeoutMs = LSM1X0A_BOOT_ALERT_TIMEOUT_MS);

  /**
   * @brief Sends a command and extracts a value from the response.
   * @param cmd Command to send (e.g., "AT+DEUI")
   * @param expectedTag Tag that precedes the value (e.g., "DevEui: ") or NULL if no tag.
   * @param outBuffer Buffer to copy the result into (does NOT include the tag).
   * @param outSize Size of the output buffer.
   * @param timeoutMs Timeout to wait for the expected tag result.
   * @return AtError
   */
  AtError sendCommandWithResponse(const char* cmd, const char* expectedTag, char* outBuffer, size_t outSize, uint32_t timeoutMs = 2000);

  /**
   * @brief Parses incoming raw data from the UART driver.
   * @param data Pointer to the incoming byte array.
   * @param len Number of bytes received.
   */
  void eatBuffer(const uint8_t* data, size_t len);

  /**
   * @brief Converts an AtError enum value into a readable string.
   * @param err The AtError to convert.
   * @return The string representation.
   */
  const char* atErrorToString(AtError err);
  /**
   * @brief Static helper to convert the metadata string into a usable struct.
   * Parses: "+EVT:RX_1, PORT 2, DR 5, RSSI -90, SNR 10, DMODM 10, GWN 2"
   */
  static bool parseRxMetadata(const char* payload, LsmRxMetadata* outMeta);

  /**
   * @brief Obtain the module type detected during boot. This is useful to adapt the controller's behavior to differences between LSM100A and LSM110A.
   * The type is determined by analyzing responses to specific AT commands during the initialization phase. If the module does not respond or cannot
   * be identified, it returns LsmModuleType::UNKNOWN.
   * @return The detected LsmModuleType (LSM100A, LSM110A, or UNKNOWN).
   */
  LsmModuleType getDeviceType() const
  {
    return _deviceType;
  }

  /**
   * @brief Obtains the operating mode detected during boot. This is useful to adapt the controller's behavior to differences between LoRaWAN and
   * SigFox modes. The mode is determined by analyzing responses to specific AT commands during the initialization phase. If the module does not
   * respond or cannot  be identified, it returns LsmMode::MODE_UNKNOWN.
   * @return The detected LsmMode (LORAWAN, SIGFOX, or MODE_UNKNOWN).
   *
   */
  LsmMode getDetectedMode() const
  {
    return _detectedMode;
  }

private:
  UartDriver*     _driver        = nullptr;
  AtEventCallback _eventCallback = nullptr;
  void*           _eventCtx      = nullptr;

  LsmModuleType _deviceType   = LsmModuleType::UNKNOWN;
  LsmMode       _detectedMode = LsmMode::MODE_UNKNOWN;

  // Internal line buffer for assembling incoming UART data until a full line is received
  char     _lineBuffer[AT_BUFFER_SIZE] = {0};
  uint16_t _lineIdx                    = 0;

  // Synchronization
  SemaphoreHandle_t _syncSem = nullptr;

  // Current transaction state
  bool    _pendingCommand  = false;
  AtError _lastResultError = AtError::UNKNOWN;

  // Pointers for "Zero-Copy" parsing during the transaction
  const char* _expectedTag   = nullptr; // The tag we expect in the response (e.g., "DevEui: ")
  char*       _userOutBuffer = nullptr; // Where the user wants us to copy the result (e.g., a local or global buffer)
  size_t      _userOutSize   = 0;       // Size of the output buffer to prevent overwrites

  // Internal methods
  void    processLine(char* line);
  AtError parseErrorString(const char* line);

  // Static bridge
  static void staticRxCallback(void* ctx, uint8_t* data, size_t len);
};

#endif // LSM1X0A_AT_PARSER_H