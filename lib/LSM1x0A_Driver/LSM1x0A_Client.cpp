#include "LSM1x0A_Client.h"

LSM1x0A_Client::LSM1x0A_Client()
{
  _controller     = new LSM1x0A_Controller();
  _configuredMode = LsmMode::LORAWAN;
  _abpConfigured  = false;
}

LSM1x0A_Client::~LSM1x0A_Client()
{
  if (_controller) {
    delete _controller;
  }
}

bool LSM1x0A_Client::begin(LsmLogCallback logCb)
{
  return begin(logCb, LsmLogLevel::INFO);
}

bool LSM1x0A_Client::begin(LsmLogCallback logCb, LsmLogLevel runtimeLevel)
{
  if (logCb) {
    _controller->setLogCallback(logCb, runtimeLevel);
  }

  if (!_controller->begin()) {
    return false;
  }

  // Attempt to wake up and sync
  return checkConnection();
}

bool LSM1x0A_Client::checkConnection()
{
  return _controller->wakeUp();
}

bool LSM1x0A_Client::setupLoRaWAN_OTAA(LsmBand band, const char* appEui, const char* appKey, LsmClass devClass)
{
  _configuredMode = LsmMode::LORAWAN;
  _abpConfigured  = false;

  if (!_controller->setMode(LsmMode::LORAWAN))
    return false;

  // Read devNonce before setting AppKey
  int devNonce = _controller->lorawan.getDevNonce();

  bool ok = true;
  if (band != LsmBand::BAND_UNKNOWN)
    ok &= _controller->lorawan.setBand(band);
  if (appEui)
    ok &= _controller->lorawan.setAppEUI(appEui);
  if (appKey) {
    ok &= _controller->lorawan.setAppKey(appKey);
    ok &= _controller->lorawan.setNwkKey(appKey); // For OTAA 1.0.4, AppKey and NwkKey are the same
  }
  ok &= _controller->lorawan.setClass(devClass);

  if (devNonce >= 0)
    ok &= _controller->lorawan.setDevNonce(devNonce);

  return ok;
}

bool LSM1x0A_Client::setupLoRaWAN_ABP(LsmBand band, const char* devAddr, const char* nwkSKey, const char* appSKey, LsmClass devClass)
{
  _configuredMode = LsmMode::LORAWAN;
  _abpConfigured  = true;

  if (!_controller->setMode(LsmMode::LORAWAN))
    return false;

  // Get current counter
  int frameCounter = _controller->lorawan.getAbpFrameCounter();

  bool ok = true;
  ok &= _controller->lorawan.setBand(band);
  ok &= _controller->lorawan.setDevAddr(devAddr);
  ok &= _controller->lorawan.setNwkSKey(nwkSKey);
  ok &= _controller->lorawan.setAppSKey(appSKey);
  ok &= _controller->lorawan.setClass(devClass);

  if (frameCounter >= 0)
    ok &= _controller->lorawan.setAbpFrameCounter(frameCounter);

  return ok;
}

bool LSM1x0A_Client::setupSigfox(LsmRCChannel rcZone)
{
  _configuredMode = LsmMode::SIGFOX;

  if (!_controller->setMode(LsmMode::SIGFOX))
    return false;

  if (rcZone != LsmRCChannel::RC_UNKNOWN) {
    return _controller->sigfox.setRcChannel(rcZone);
  }

  return true;
}

bool LSM1x0A_Client::joinNetwork()
{
  _joinStatus = LsmJoinStatus::JOIN_IN_PROCESS;

  if (_configuredMode == LsmMode::LORAWAN) {
    if (_abpConfigured) {
      if (_controller->lorawan.join(LsmJoinMode::ABP)) {
        _joinStatus = LsmJoinStatus::JOINED;
        return true;
      }
    }
    else {
      for (uint8_t i = 0; i < 5; i++) {
        if (_controller->lorawan.join(LsmJoinMode::OTAA)) {
          _joinStatus = LsmJoinStatus::JOINED;
          return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    for (uint8_t i = 0; i < 5; i++) {
      if (_controller->sigfox.join()) {
        _joinStatus = LsmJoinStatus::JOINED;
        return true;
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  _joinStatus = LsmJoinStatus::NOT_JOINED;
  return false;
}

LsmJoinStatus LSM1x0A_Client::isJoined()
{
  if (_joinStatus == LsmJoinStatus::JOIN_IN_PROCESS) {
    return LsmJoinStatus::JOIN_IN_PROCESS;
  }

  if (_configuredMode == LsmMode::LORAWAN) {
    return _controller->lorawan.isJoined() ? LsmJoinStatus::JOINED : LsmJoinStatus::NOT_JOINED;
  }

  return LsmJoinStatus::JOINED; // Sigfox is considered "ready/joined" if configured and not explicitly joining
}

bool LSM1x0A_Client::sendPayload(const uint8_t* payload, size_t length, bool requestAck, uint8_t port, bool enableRetries, uint8_t maxRetries)
{
  if (!payload || length == 0)
    return false;

  uint8_t attempts = enableRetries ? (maxRetries + 1) : 1;
  bool    success  = false;

  if (_configuredMode == LsmMode::LORAWAN) {
    if (isJoined() == LsmJoinStatus::NOT_JOINED) {
      LSM_LOG_WARN("CLIENT", "Not joined to LoRaWAN, attempting auto-reconnect...");
      if (!joinNetwork()) {
        LSM_LOG_ERROR("CLIENT", "Auto-reconnect failed, discarding payload.");
        return false;
      }
    }

    // Convert to hex string
    size_t safeLength = length;
    if (safeLength > 242)
      safeLength = 242; // Prevent overflow on 512 byte buffer (max LoRaWAN payload is 242 anyways)
    char hexString[512];
    for (size_t j = 0; j < safeLength; j++) {
      sprintf(&hexString[j * 2], "%02X", payload[j]);
    }
    hexString[safeLength * 2] = '\0';

    for (uint8_t i = 0; i < attempts; i++) {
      success = _controller->lorawan.sendData(port, hexString, requestAck);
      if (success)
        return true;

      if (requestAck && enableRetries && i == attempts - 1) {
        _controller->recoverModule();
      }
      else if (!success && i < attempts - 1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    for (uint8_t i = 0; i < attempts; i++) {
      success = _controller->sigfox.sendPayload(payload, length, requestAck, 2);
      if (success)
        return true;
      if (!success && i < attempts - 1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
  }

  return false;
}

bool LSM1x0A_Client::sendString(const char* text, bool requestAck, uint8_t port, bool enableRetries, uint8_t maxRetries)
{
  if (!text)
    return false;

  uint8_t attempts = enableRetries ? (maxRetries + 1) : 1;
  bool    success  = false;

  if (_configuredMode == LsmMode::LORAWAN) {
    if (isJoined() == LsmJoinStatus::NOT_JOINED) {
      LSM_LOG_WARN("CLIENT", "Not joined to LoRaWAN, attempting auto-reconnect...");
      if (!joinNetwork()) {
        LSM_LOG_ERROR("CLIENT", "Auto-reconnect failed, discarding string.");
        return false;
      }
    }

    size_t length = strlen(text);
    if (length > 242)
      length = 242;
    char hexString[512];
    for (size_t i = 0; i < length; i++) {
      sprintf(&hexString[i * 2], "%02X", (uint8_t)text[i]);
    }
    hexString[length * 2] = '\0';

    for (uint8_t i = 0; i < attempts; i++) {
      success = _controller->lorawan.sendData(port, hexString, requestAck);
      if (success)
        return true;

      if (requestAck && enableRetries && i == attempts - 1) {
        _controller->recoverModule();
      }
      else if (!success && i < attempts - 1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    for (uint8_t i = 0; i < attempts; i++) {
      success = _controller->sigfox.sendString(text, requestAck, 2);
      if (success)
        return true;
      if (!success && i < attempts - 1) {       
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }
  }
  return false;
}

int LSM1x0A_Client::getBatteryVoltage()
{
  return _controller->getBattery();
}

LsmModuleType LSM1x0A_Client::getModuleType()
{
  return _controller->getDeviceType();
}

bool LSM1x0A_Client::getFirmwareVersion(char* buffer, size_t size)
{
  return _controller->getVersion(buffer, size);
}

int LSM1x0A_Client::getLastRssi() const
{
  if (_configuredMode == LsmMode::LORAWAN) {
    return _controller->getLastRssi();
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    return _controller->sigfox.getLastRxRSSI();
  }
  return 0;
}

int LSM1x0A_Client::getLastSnr() const
{
  if (_configuredMode == LsmMode::LORAWAN) {
    return _controller->getLastSnr();
  }
  return 0;
}

LSM1x0A_Controller& LSM1x0A_Client::getController()
{
  return *_controller;
}

// Internal hex string parser, in case we need to reconstruct bytes from hex downlinks
bool LSM1x0A_Client::_charsToBytes(const char* hexString, uint8_t* outputBuffer, size_t maxLen)
{
  if (!hexString || !outputBuffer)
    return false;
  size_t hexLen = strlen(hexString);
  if (hexLen % 2 != 0 || (hexLen / 2) > maxLen)
    return false;

  for (size_t i = 0; i < hexLen / 2; i++) {
#ifdef _MSC_VER
    sscanf_s(&hexString[i * 2], "%2hhx", &outputBuffer[i]);
#else
    sscanf(&hexString[i * 2], "%2hhx", &outputBuffer[i]);
#endif
  }
  return true;
}

void LSM1x0A_Client::setDownlinkCallback(LsmDownlinkCallback callback)
{
  _downlinkCallback = callback;
  if (_downlinkCallback) {
    _controller->setRxCallback(_onRxData, this);
  }
  else {
    _controller->setRxCallback(nullptr, nullptr);
  }
}

void LSM1x0A_Client::_onRxData(void* ctx, const char* payload)
{
  LSM1x0A_Client* self = static_cast<LSM1x0A_Client*>(ctx);
  if (!self || !self->_downlinkCallback || !payload)
    return;

  char temp[256];
  strncpy(temp, payload, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';

  char* portStr = strtok(temp, ":");
  char* sizeStr = strtok(NULL, ":");
  char* dataStr = strtok(NULL, ":");

  if (!portStr || !sizeStr || !dataStr)
    return;

  uint8_t port = (uint8_t)strtol(portStr, NULL, 10);
  size_t  size = (size_t)strtol(sizeStr, NULL, 16);

  uint8_t binPayload[128];
  if (size > sizeof(binPayload))
    size = sizeof(binPayload);

  if (self->_charsToBytes(dataStr, binPayload, size)) {
    self->_downlinkCallback(port, binPayload, size);
  }
}
