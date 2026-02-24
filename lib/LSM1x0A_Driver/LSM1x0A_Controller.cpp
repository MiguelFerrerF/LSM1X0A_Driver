#include "LSM1x0A_Controller.h"
#include <Arduino.h>

LSM1x0A_Controller::LSM1x0A_Controller() : lorawan(this)
{
  _driver      = new UartDriver();
  _parser      = new LSM1x0A_AtParser();
  _initialized = false;
  _isJoined    = false;
  _resetPin    = LSM1X0A_RESET_PIN;
  _maxRetries  = DEFAULT_MAX_RETRIES;

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

bool LSM1x0A_Controller::begin(AtEventCallback callback, void* ctx)
{
  if (_initialized)
    return true;

  if (!_driver || !_parser) {
    return false;
  }

  _userCallback = callback;
  _userCtx      = ctx;

  // Inicializamos el parser pasándole el UartDriver que construimos.
  // El parser internamente llama a _driver->init()
  // Pasamos internalEventCallback como callback, y apuntamos ctx a 'this'
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

    // Errores definitorios por los que NO tiene sentido reintentar (sintaxis mala, etc)
    if (err == AtError::PARAM_ERROR || err == AtError::TEST_PARAM_OVERFLOW || err == AtError::NO_NET_JOINED) {
      return err;
    }

    // Si no es el último intento, hacemos una breve pausa
    if (i < retries - 1) {
      delay(200);
    }
  }

  // Si agotamos todos los reintentos (ej: timeouts constantes), ejecutamos recuperación
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
    outBuffer[0] = '\0'; // Limpiar buffer en cada intento
    err          = _parser->sendCommandWithResponse(cmd, expectedTag, outBuffer, outSize, timeoutMs);
    if (err == AtError::OK)
      return err;

    if (err == AtError::PARAM_ERROR || err == AtError::TEST_PARAM_OVERFLOW || err == AtError::NO_NET_JOINED) {
      return err;
    }

    if (i < retries - 1) {
      delay(200);
    }
  }

  // Si falló persistentemente
  recoverModule();
  return err;
}

// =========================================================================
// COMANDOS AT BÁSICOS / GENERALES
// =========================================================================

int LSM1x0A_Controller::getBattery()
{
  char buf[16];
  if (sendCommandWithResponse(LsmAtCommand::BATTERY, buf, sizeof(buf), nullptr, 1000) != AtError::OK) {
    return -1;
  }
  // Convertir de char "3300" a int
  return atoi(buf);
}

bool LSM1x0A_Controller::getVersion(char* outBuffer, size_t size)
{
  if (!outBuffer || size == 0)
    return false;

  // Comando AT+VER=?
  // Puede venir con "APP_VERSION:" o nada, este módulo devuelve múltiples líneas
  // usaremos el base parser que agarra la respuesta principal.
  AtError err = sendCommandWithResponse(LsmAtCommand::FW_VERSION, outBuffer, size, "APP_VERSION:", 2000);

  // Si la etiqueta APP_VERSION no se encontraba, capturamos todo
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

bool LSM1x0A_Controller::getLocalTime(struct tm* timeinfo)
{
  if (!timeinfo)
    return false;

  char buffer[64];
  if (sendCommandWithResponse(LsmAtCommand::LOCAL_TIME, buffer, sizeof(buffer), "LTIME:", 1000) != AtError::OK)
    return false;

  // El parser remueve los prefijos, por lo que buffer debería contener algo como "12h34m56s on 23/02/2026"
  int h = 0, m = 0, s = 0, D = 0, M = 0, Y = 0;
  if (sscanf(buffer, "%dh%dm%ds on %d/%d/%d", &h, &m, &s, &D, &M, &Y) == 6) {
    timeinfo->tm_hour = h;
    timeinfo->tm_min  = m;
    timeinfo->tm_sec  = s;
    timeinfo->tm_mday = D;
    timeinfo->tm_mon  = M - 1;    // struct tm usa 0-11 para los meses
    timeinfo->tm_year = Y - 1900; // struct tm usa años desde 1900
    return true;
  }

  return false;
}

int LSM1x0A_Controller::getBaudrate()
{

  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::BAUDRATE);
  char buf[32];
  if (sendCommandWithResponse(cmd, buf, sizeof(buf), "Set BaudRate: ", 1000) != AtError::OK) {
    return -1;
  }

  return atoi(buf);
}

bool LSM1x0A_Controller::setBaudrate(uint32_t baudrate)
{
  if (baudrate != 9600 && baudrate != 115200) {
    return false;
  }
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::BAUDRATE, baudrate);
  return sendCommand(cmd, 1000) == AtError::OK;
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
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::MODE, (int)mode);
  AtError err = _parser->sendCommand(cmd, LSM1X0A_BOOT_ALERT_TIMEOUT_MS);
  if (err == AtError::BOOT_ALERT) {
    return wakeUp();
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

bool LSM1x0A_Controller::softwareReset()
{
  if (!_initialized || !_parser)
    return false;
  AtError err = _parser->sendCommand(LsmAtCommand::RESET, LSM1X0A_BOOT_ALERT_TIMEOUT_MS);
  if (err == AtError::BOOT_ALERT) {
    return wakeUp();
  }
  return false;
}

bool LSM1x0A_Controller::hardwareReset()
{
  if (!_initialized || !_parser || _resetPin < 0)
    return false;

  pinMode(_resetPin, OUTPUT);
  digitalWrite(_resetPin, LOW);
  delay(100);
  digitalWrite(_resetPin, HIGH);

  AtError err = _parser->waitForEvent(LSM1X0A_BOOT_ALERT_TIMEOUT_MS);
  if (err == AtError::BOOT_ALERT) {
    delay(1000); // Esperar a que el bootloader termine completamente
    return wakeUp();
  }
  return false;
}

bool LSM1x0A_Controller::recoverModule()
{

  // 1. Intento Software (ATZ) con reintentos
  for (int i = 0; i < _maxRetries; i++) {
    if (softwareReset())
      return true;
    delay(500);
  }

  // 2. Intento Hardware (Si el pin está configurado) con reintentos
  if (_resetPin >= 0) {
    for (int i = 0; i < _maxRetries; i++) {
      if (hardwareReset())
        return true;
    }
  }

  return false;
}

// =========================================================================
// ESTADO Y SINCRONIZACIÓN NATIVA
// =========================================================================

bool LSM1x0A_Controller::isJoined() const
{
  return _isJoined;
}

uint32_t LSM1x0A_Controller::waitForEvent(uint32_t bitsToWaitFor, uint32_t timeoutMs, bool clearOnExit)
{
  if (!_syncEventGroup) return 0;
  return xEventGroupWaitBits(_syncEventGroup, bitsToWaitFor, clearOnExit ? pdTRUE : pdFALSE, pdFALSE, pdMS_TO_TICKS(timeoutMs));
}

void LSM1x0A_Controller::clearEvents(uint32_t bitsToClear)
{
  if (!_syncEventGroup) return;
  xEventGroupClearBits(_syncEventGroup, bitsToClear);
}

// =========================================================================
// INTERCEPTOR DE EVENTOS ASÍNCRONOS
// =========================================================================

void LSM1x0A_Controller::internalEventCallback(const char* type, const char* payload, void* ctx)
{
  if (!ctx) return;
  LSM1x0A_Controller* self = static_cast<LSM1x0A_Controller*>(ctx);
  self->handleEvent(type, payload);
}

void LSM1x0A_Controller::handleEvent(const char* type, const char* payload)
{
  // 1. Interceptar para cambiar el estado interno y liberar semáforos
  if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strstr(payload, "SUCCESS") || strstr(payload, "Network joined")) {
      _isJoined = true;
      if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_JOIN_SUCCESS);
    } 
    else if (strstr(payload, "FAILED") || strstr(payload, "Join failed")) {
      _isJoined = false;
      if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_JOIN_FAIL);
    }
  }
  else if (strcmp(type, LsmEvent::TX) == 0) {
    if (strstr(payload, "SUCCESS")) {
      if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_TX_SUCCESS);
    }
    else if (strstr(payload, "FAILED") || strstr(payload, "TIMEOUT")) {
      if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_TX_FAIL);
    }
  }
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_RX_DATA);
  }
  else if (strcmp(type, LsmEvent::RX_META) == 0) {
    if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_TX_SUCCESS);
  } 
  else if (strcmp(type, LsmEvent::RX_TIMEOUT) == 0) {
    if (_syncEventGroup) xEventGroupSetBits(_syncEventGroup, LSM_EVT_RX_TIMEOUT);
  }

  // 2. Pasar el evento hacia arriba al callback del usuario
  if (_userCallback) {
    _userCallback(type, payload, _userCtx);
  }
}

