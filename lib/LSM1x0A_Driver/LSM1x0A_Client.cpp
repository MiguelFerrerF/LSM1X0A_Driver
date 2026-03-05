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
  if (logCb) {
    _controller->setLogCallback(logCb);
  }

  if (!_controller->begin()) {
    return false;
  }

  // Attempt to wake up and sync
  return _controller->wakeUp();
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
  ok &= _controller->lorawan.setBand(band);
  ok &= _controller->lorawan.setAppEUI(appEui);
  ok &= _controller->lorawan.setAppKey(appKey);
  ok &= _controller->lorawan.setNwkKey(appKey); // For OTAA 1.0.4, AppKey and NwkKey are the same
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

  return _controller->sigfox.setRcChannel(rcZone);
}

bool LSM1x0A_Client::joinNetwork()
{
  if (_configuredMode == LsmMode::LORAWAN) {
    if (_abpConfigured) {
      return _controller->lorawan.join(LsmJoinMode::ABP);
    }
    else {
      for (uint8_t i = 0; i < 5; i++) {
        if (_controller->lorawan.join(LsmJoinMode::OTAA))
          return true;
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      return false;
    }
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    for (uint8_t i = 0; i < 5; i++) {
      if (_controller->sigfox.join())
        return true;
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    return false;
  }
  return false;
}

bool LSM1x0A_Client::isJoined()
{
  if (_configuredMode == LsmMode::LORAWAN) {
    return _controller->lorawan.isJoined();
  }
  return true; // Sigfox is considered "ready/joined" if configured
}

bool LSM1x0A_Client::sendPayload(const uint8_t* payload, size_t length, bool requestAck, uint8_t port)
{
  if (!payload || length == 0)
    return false;

  if (_configuredMode == LsmMode::LORAWAN) {
    // Convert to hex string
    if (length > 242)
      length = 242; // Prevent overflow on 512 byte buffer (max LoRaWAN payload is 242 anyways)
    char hexString[512];
    for (size_t i = 0; i < length; i++) {
      sprintf(&hexString[i * 2], "%02X", payload[i]);
    }
    hexString[length * 2] = '\0';
    return _controller->lorawan.sendData(port, hexString, requestAck);
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    return _controller->sigfox.sendPayload(payload, length, requestAck, 2);
  }

  return false;
}

bool LSM1x0A_Client::sendString(const char* text, bool requestAck, uint8_t port)
{
  if (!text)
    return false;

  if (_configuredMode == LsmMode::LORAWAN) {
    size_t length = strlen(text);
    if (length > 242)
      length = 242;
    char hexString[512];
    for (size_t i = 0; i < length; i++) {
      sprintf(&hexString[i * 2], "%02X", (uint8_t)text[i]);
    }
    hexString[length * 2] = '\0';
    return _controller->lorawan.sendData(port, hexString, requestAck);
  }
  else if (_configuredMode == LsmMode::SIGFOX) {
    return _controller->sigfox.sendString(text, requestAck, 2);
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
