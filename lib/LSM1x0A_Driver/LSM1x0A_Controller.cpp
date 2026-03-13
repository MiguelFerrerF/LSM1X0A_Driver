#include "LSM1x0A_Controller.h"

LSM1x0A_Controller::LSM1x0A_Controller() : lorawan(this), sigfox(this)
{
  _driver      = new UartDriver();
  _parser      = new LSM1x0A_AtParser();
  _initialized = false;
  _resetPin    = LSM1X0A_RESET_PIN;
  _maxRetries  = DEFAULT_MAX_RETRIES;

  _lastRssi  = 0;
  _lastSnr   = 0;
  _lastDmodm = 0;
  _lastGwn   = 0;

  _syncEventGroup = xEventGroupCreate();
}

LSM1x0A_Controller::~LSM1x0A_Controller()
{
  end();
  if (_parser) {
    delete _parser;
    _parser = nullptr;
  }
  if (_syncEventGroup) {
    vEventGroupDelete(_syncEventGroup);
    _syncEventGroup = nullptr;
  }
  if (_driver) {
    delete _driver;
    _driver = nullptr;
  }
}

bool LSM1x0A_Controller::begin()
{
  if (_initialized)
    return true;

  if (!_driver || !_parser) {
    return false;
  }

  // Initialize the AT parser by passing the UartDriver we constructed.
  // The parser internally calls _driver->init()
  // We pass internalEventCallback as the callback, and point ctx to 'this' controller so we can route events back to the user callback.
  if (!_parser->init(_driver, internalEventCallback, this)) {
    return false;
  }

  _initialized = true;
  return true;
}

void LSM1x0A_Controller::end()
{
  if (!_initialized)
    return;

  if (_driver) {
    _driver->deinit();
  }
  _initialized = false;
}

bool LSM1x0A_Controller::wakeUp()
{
  if (!_initialized || !_parser)
    return false;
  return _parser->wakeUp();
}

AtError LSM1x0A_Controller::sendCommand(const char* cmd, uint32_t timeoutMs, int8_t retries)
{
  if (!_initialized || !_parser)
    return AtError::GENERIC_ERROR;

  if (retries < 0) {
    retries = _maxRetries;
  }

  AtError err = AtError::GENERIC_ERROR;
  for (int i = 0; i < retries; i++) {
    err = _parser->sendCommand(cmd, timeoutMs);

    if (err == AtError::OK)
      return err;

    // Errors that indicate no point in retrying (bad syntax, regulatory limits, etc)
    if (err == AtError::PARAM_ERROR || err == AtError::TEST_PARAM_OVERFLOW || err == AtError::NO_NET_JOINED || err == AtError::DUTY_CYCLE_RESTRICT) {
      return err;
    }

    if (err == AtError::BUSY) {
      LSM_LOG_WARN("CTRL", "Module is busy, retrying command (%d/%d)", i + 1, retries);
      vTaskDelay(pdMS_TO_TICKS(5000)); // Wait longer for busy state
      continue;
    }

    // If not the last attempt, make a brief pause
    if (i < retries - 1) {
      LSM_LOG_WARN("CTRL", "sendCommand retrying (%d/%d)", i + 1, retries);
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }

  // If all retries are exhausted (e.g., constant timeouts), perform recovery
  LSM_LOG_ERROR("CTRL", "sendCommand failed after %d retries", retries);
  recoverModule();
  return err;
}

AtError LSM1x0A_Controller::sendCommandWithResponse(const char* cmd, char* outBuffer, size_t outSize, const char* expectedTag, uint32_t timeoutMs,
                                                    int8_t retries)
{
  if (!_initialized || !_parser)
    return AtError::GENERIC_ERROR;

  if (!outBuffer || outSize == 0)
    return AtError::PARAM_ERROR;

  if (retries < 0) {
    retries = _maxRetries;
  }

  AtError err = AtError::GENERIC_ERROR;
  for (int i = 0; i < retries; i++) {
    outBuffer[0] = '\0'; // Clear buffer on each attempt
    err          = _parser->sendCommandWithResponse(cmd, expectedTag, outBuffer, outSize, timeoutMs);
    if (err == AtError::OK)
      return err;

    if (err == AtError::PARAM_ERROR || err == AtError::TEST_PARAM_OVERFLOW || err == AtError::NO_NET_JOINED || err == AtError::DUTY_CYCLE_RESTRICT) {
      return err;
    }

    if (err == AtError::BUSY) {
      LSM_LOG_WARN("CTRL", "Module is busy, retrying command (%d/%d)", i + 1, retries);
      vTaskDelay(pdMS_TO_TICKS(5000)); // Wait longer for busy state
      continue;
    }

    if (i < retries - 1) {
      LSM_LOG_WARN("CTRL", "sendCommandWithResponse retrying (%d/%d)", i + 1, retries);
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }

  // If it failed persistently
  LSM_LOG_ERROR("CTRL", "sendCommandWithResponse failed after %d retries", retries);
  recoverModule();
  return err;
}

// =========================================================================
// BASIC / GENERAL AT COMMANDS
// =========================================================================

int LSM1x0A_Controller::getBattery()
{
  char buf[16];
  if (sendCommandWithResponse(LsmAtCommand::BATTERY, buf, sizeof(buf), nullptr, 1000) != AtError::OK) {
    return -1;
  }
  // Convert from char "3300" to int
  return atoi(buf);
}

bool LSM1x0A_Controller::getVersion(char* outBuffer, size_t size)
{
  if (!outBuffer || size == 0)
    return false;

  // AT+VER=? command
  // It may come with "APP_VERSION:" or nothing, this module returns multiple lines
  // We use the base parser which captures the main response.
  AtError err = sendCommandWithResponse(LsmAtCommand::FW_VERSION, outBuffer, size, "APP_VERSION:", 2000);

  // If the APP_VERSION tag was not found, capture everything
  if (err != AtError::OK) {
    err = sendCommandWithResponse(LsmAtCommand::FW_VERSION, outBuffer, size, nullptr, 2000);
  }
  return err == AtError::OK;
}

bool LSM1x0A_Controller::factoryReset()
{
  return sendCommand(LsmAtCommand::FACTORY_RESET, LSM1X0A_BOOT_ALERT_TIMEOUT_MS) == AtError::OK;
}

bool LSM1x0A_Controller::getSigfoxVersion(char* buffer, size_t size)
{
  if (!buffer || size == 0)
    return false;
  AtError err = sendCommandWithResponse(LsmAtCommand::SSW_VERSION, buffer, size, "SW_VERSION:", 2000);
  return err == AtError::OK;
}

bool LSM1x0A_Controller::setVerboseLevel(uint8_t level)
{
  if (level > 3)
    return false;
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::VERBOSE_LEVEL, level);
  AtError err = sendCommand(cmd, 2000);
  return err == AtError::OK;
}

bool LSM1x0A_Controller::setMode(LsmMode mode)
{
  if (!_initialized || !_parser)
    return false;

  // Optimization: if we already know it's in the desired mode, don't send anything
  if (_parser->getDetectedMode() == mode) {
    _currentMode = mode;
    return true; // Already in the desired mode
  }

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::MODE, (int)mode);

  AtError err = _parser->sendCommand(cmd, LSM1X0A_BOOT_ALERT_TIMEOUT_MS);

  // Validate that it triggered a reboot AND that the parser identified the correct mode
  if (err == AtError::BOOT_ALERT && _parser->getDetectedMode() == mode) {
    _currentMode = mode;
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// CONFIGURATION GETTERS
// ------------------------------------------------------------------

LsmModuleType LSM1x0A_Controller::getDeviceType() const
{
  if (!_initialized || !_parser)
    return LsmModuleType::UNKNOWN;
  return _parser->getDeviceType();
}

void LSM1x0A_Controller::setResetPin(int pin)
{
  _resetPin = pin;
}

void LSM1x0A_Controller::setMaxRetries(int retries)
{
  _maxRetries = (retries > 0) ? retries : 1;
}

void LSM1x0A_Controller::setLogCallback(LsmLogCallback callback, LsmLogLevel runtimeLevel)
{
  LsmLogger::setCallback(callback);
  LsmLogger::setLevel(runtimeLevel);
  LSM_LOG_DEBUG("CTRL", "Logger callback registered, level: %d", (int)runtimeLevel);
}

void LSM1x0A_Controller::setRxCallback(LsmRxCallback callback, void* ctx)
{
  _rxCallback = callback;
  _rxCtx      = ctx;
}

bool LSM1x0A_Controller::softwareReset()
{
  if (!_initialized || !_parser)
    return false;
  AtError err = _parser->sendCommand(LsmAtCommand::RESET, LSM1X0A_BOOT_ALERT_TIMEOUT_MS);
  if (err == AtError::BOOT_ALERT) {
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::hardwareReset()
{
  if (!_initialized || !_parser || _resetPin < 0)
    return false;

  gpio_reset_pin((gpio_num_t)_resetPin);
  gpio_set_direction((gpio_num_t)_resetPin, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)_resetPin, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level((gpio_num_t)_resetPin, 1);

  AtError err = _parser->waitForEvent(LSM1X0A_BOOT_ALERT_TIMEOUT_MS);
  if (err == AtError::BOOT_ALERT) {
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::recoverModule(bool joinAfterRecovery)
{
  LSM_LOG_WARN("CTRL", "Starting module recovery procedure...");

  bool wasJoined = false;
  if (joinAfterRecovery || lorawan.isJoined())
    wasJoined = true;
  bool isRecovered = false;

  // 1. Software attempt (ATZ) with retries
  for (int i = 0; i < _maxRetries; i++) {
    if (softwareReset()) {
      isRecovered = true;
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // 2. Hardware attempt (if the pin is configured) with retries
  if (!isRecovered && _resetPin >= 0) {
    for (int i = 0; i < _maxRetries; i++) {
      if (hardwareReset()) {
        isRecovered = true;
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }

  if (isRecovered) {
    if (_currentMode == LsmMode::LORAWAN) {
      lorawan.setJoined(false);
      // Reconfigure the module parameters
      if (lorawan.restoreConfig()) {
        if (wasJoined) {
          lorawan.recoverConnection(_maxRetries);
        }
        else {
          clearEvents(LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL | LSM_EVT_TX_SUCCESS | LSM_EVT_TX_FAIL | LSM_EVT_RX_DATA | LSM_EVT_RX_TIMEOUT);
        }
      }
    }
  }

  return isRecovered;
}

// =========================================================================
// NATIVE STATE AND SYNCHRONIZATION
// =========================================================================

uint32_t LSM1x0A_Controller::waitForEvent(uint32_t bitsToWaitFor, uint32_t timeoutMs, bool clearOnExit)
{
  if (!_syncEventGroup)
    return 0;
  return xEventGroupWaitBits(_syncEventGroup, bitsToWaitFor, clearOnExit ? pdTRUE : pdFALSE, pdFALSE, pdMS_TO_TICKS(timeoutMs));
}

void LSM1x0A_Controller::clearEvents(uint32_t bitsToClear)
{
  if (!_syncEventGroup)
    return;
  xEventGroupClearBits(_syncEventGroup, bitsToClear);
}

// =========================================================================
// ASYNCHRONOUS EVENT INTERCEPTOR
// =========================================================================

void LSM1x0A_Controller::internalEventCallback(const char* type, const char* payload, void* ctx)
{
  if (!ctx)
    return;
  LSM1x0A_Controller* self = static_cast<LSM1x0A_Controller*>(ctx);
  self->handleEvent(type, payload);
}

void LSM1x0A_Controller::handleEvent(const char* type, const char* payload)
{
  LSM_LOG_VERBOSE("CTRL", "Received internal event: %s -> %s", type, payload);
  // 1. Intercept to change internal state and release semaphores
  if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strstr(payload, "SUCCESS") || strstr(payload, "Network joined")) {
      lorawan.setJoined(true);
      if (_syncEventGroup)
        xEventGroupSetBits(_syncEventGroup, LSM_EVT_JOIN_SUCCESS);
    }
    else if (strstr(payload, "FAILED") || strstr(payload, "Join failed")) {
      lorawan.setJoined(false);
      if (_syncEventGroup)
        xEventGroupSetBits(_syncEventGroup, LSM_EVT_JOIN_FAIL);
    }
  }
  else if (strcmp(type, LsmEvent::TX) == 0) {
    if (strstr(payload, "SUCCESS")) {
      if (_syncEventGroup)
        xEventGroupSetBits(_syncEventGroup, LSM_EVT_TX_SUCCESS);
      lorawan.setJoined(true);
    }
    else if (strstr(payload, "FAILED") || strstr(payload, "TIMEOUT")) {
      if (_syncEventGroup)
        xEventGroupSetBits(_syncEventGroup, LSM_EVT_TX_FAIL);
    }
  }
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    if (_syncEventGroup)
      xEventGroupSetBits(_syncEventGroup, LSM_EVT_RX_DATA);
    lorawan.setJoined(true);
    if (_rxCallback) {
      _rxCallback(_rxCtx, payload);
    }
  }
  else if (strcmp(type, LsmEvent::RX_META) == 0) {
    uint32_t bitsToSet = LSM_EVT_TX_SUCCESS;

    LsmRxMetadata meta;
    if (LSM1x0A_AtParser::parseRxMetadata(payload, &meta)) {
      _lastRssi = meta.rssi;
      _lastSnr  = meta.snr;
      if (meta.hasLinkCheck) {
        _lastDmodm = meta.demodMargin;
        _lastGwn   = meta.nbGateways;
        bitsToSet |= LSM_EVT_LINK_CHECK_ANS;
      }
    }

    if (_syncEventGroup)
      xEventGroupSetBits(_syncEventGroup, bitsToSet);
  }
  else if (strcmp(type, LsmEvent::RX_TIMEOUT) == 0) {
    if (_syncEventGroup)
      xEventGroupSetBits(_syncEventGroup, LSM_EVT_RX_TIMEOUT);
  }
  else if (strcmp(type, LsmEvent::CHMASK) == 0) {
    if (_tempMaskCount < 6) {
      const char* hexPtr = strstr(payload, "0x");
      if (hexPtr) {
        _tempMaskBuffer[_tempMaskCount] = (uint16_t)strtol(hexPtr + 2, NULL, 16);
        _tempMaskCount++;
      }
    }
  }

  // 2. Log the event locally
  LSM_LOG_DEBUG("EVENT", "Type: %s | Payload: %s", type, payload ? payload : "N/A");
}

bool LSM1x0A_Controller::syncConfigToCache()
{
  if (!_initialized || !_parser)
    return false;

  bool success = true;
  if (_currentMode == LsmMode::SIGFOX) {
    success &= sigfox.loadConfigFromModule();
  }
  else {
    success &= lorawan.loadConfigFromModule();
  }
  return success;
}