#include "LSM1x0A_Controller.h"
#include "driver/gpio.h"

LSM1x0A_Controller::LSM1x0A_Controller()
{
  _isJoined        = false;
  _isTxBusy        = false;
  _userCallback    = nullptr;
  _currentLogLevel = LsmLogLevel::INFO; // Default: Solo información útil
}

void LSM1x0A_Controller::setLogLevel(LsmLogLevel level)
{
  _currentLogLevel = level;
}

// ---------------------------------------------------------
// SISTEMA DE LOGGING INTERNO
// ---------------------------------------------------------
void LSM1x0A_Controller::log(LsmLogLevel level, const char* format, ...)
{
  // 1. Filtrado Eficiente: Si el nivel del mensaje es superior al configurado, ignorar.
  if (level > _currentLogLevel)
    return;
  if (!_userCallback)
    return;

  // 2. Formateo de String (estilo printf)
  char    buffer[128]; // Buffer seguro para logs
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // 3. Envío al Callback usando el canal LOG
  _userCallback(LsmEvent::LOG, buffer);
}

// ---------------------------------------------------------
// RESET HARDWARE
// ---------------------------------------------------------
bool LSM1x0A_Controller::hardReset()
{
  if (_resetPin < 0)
    return false; // No configurado

  gpio_num_t pin = (gpio_num_t)_resetPin;

  log(LsmLogLevel::INFO, "Ejecutando HARD RESET (GPIO %d)...", _resetPin);

  gpio_reset_pin(pin); // Aseguramos estado limpio del pin
  gpio_set_direction(pin, GPIO_MODE_OUTPUT);
  gpio_set_level(pin, 0); // Bajar a GND
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(pin, 1); // Subir a VCC
  return true;
}

bool LSM1x0A_Controller::softReset()
{
  log(LsmLogLevel::INFO, "Ejecutando SOFT RESET vía comando AT...");

  AtError res = _parser.sendCommand("ATZ", 3000);
  if (res != AtError::BOOT_ALERT) {
    log(LsmLogLevel::ERROR, "Fallo en SOFT RESET: El módulo no respondió correctamente (Error: %s)", _parser.atErrorToString(res));
    return false;
  }
  log(LsmLogLevel::INFO, "SOFT RESET exitoso, módulo reiniciado.");
  return true;
}

// ---------------------------------------------------------
// INICIALIZACIÓN
// ---------------------------------------------------------
bool LSM1x0A_Controller::begin(UartDriver* driver, int resetPin, LsmUserCallback callback)
{
  _userCallback = callback;
  _resetPin     = resetPin;

  log(LsmLogLevel::INFO, "Iniciando Controlador LSM1x0A...");

  if (!_parser.init(driver, LSM1x0A_Controller::parserEventProxy, this)) {
    log(LsmLogLevel::ERROR, "Fallo crítico: No se pudo iniciar Parser/Driver");
    return false;
  }
  else
    log(LsmLogLevel::DEBUG, "Parser y Driver UART iniciados correctamente.");

  log(LsmLogLevel::DEBUG, "Sincronizando UART...");
  // Pequeña espera para estabilizar la UART
  vTaskDelay(100);

  int  retries = 0;
  bool synced  = false;
  while (retries < 5) {
    if (softReset()) {
      synced = true;
      break;
    }
    vTaskDelay(200);
    retries++;
  }

  if (!synced) {
    log(LsmLogLevel::ERROR, "Timeout: El módulo no responde a 'ATZ', forzando reinicio.");
    if (!hardReset()) {
      log(LsmLogLevel::ERROR, "Fallo crítico: No se pudo forzar reset hardware, pin no configurado.");
      return false;
    }
    if (!softReset()) {
      log(LsmLogLevel::ERROR, "Fallo crítico: No se pudo sincronizar con el módulo tras reset.");
      return false;
    }
    log(LsmLogLevel::DEBUG, "Sincronización exitosa tras reset.");
  }
  log(LsmLogLevel::INFO, "Controlador LSM1x0A iniciado correctamente.");
  return true;
}

// ---------------------------------------------------------
// GESTIÓN DE EVENTOS
// ---------------------------------------------------------
void LSM1x0A_Controller::parserEventProxy(const char* type, const char* payload, void* ctx)
{
  // Recuperamos la instancia del controlador
  LSM1x0A_Controller* self = (LSM1x0A_Controller*)ctx;
  self->handleInternalEvent(type, payload);
}

void LSM1x0A_Controller::handleInternalEvent(const char* type, const char* payload)
{
  // 1. Actualizar Estado Interno (State Machine)

  // --- VERBOSE LOGGING ---
  if (strcmp(type, LsmEvent::VERBOSE) == 0) {
    log(LsmLogLevel::VERBOSE, ">> %s", payload);
  }

  // --- JOIN ---
  if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      _isJoined = true;
      _isTxBusy = false; // El join libera el canal
    }
    else {
      _isJoined = false; // Falló join
      _isTxBusy = false;
    }
  }

  // --- TX DONE ---
  else if (strcmp(type, LsmEvent::TX) == 0) {
    // Ya sea SUCCESS o FAILED, la transmisión terminó
    _isTxBusy = false;
  }

  // --- RX DATA (Downlink) ---
  // A veces recibir un downlink implica que la ventana RX2 cerró y estamos libres
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    _isTxBusy = false;
  }

  // 2. Propagar al usuario
  if (_userCallback) {
    _userCallback(type, payload);
  }
}

// ---------------------------------------------------------
// MÉTODOS PÚBLICOS
// ---------------------------------------------------------
bool LSM1x0A_Controller::join(bool isOTAA)
{
  // Si ya estamos uniendo o transmitiendo, no interrumpir
  if (_isTxBusy)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+JOIN=%d", isOTAA ? 1 : 0);
  AtError err = _parser.sendCommand(cmd);
  if (err == AtError::OK) {
    _isTxBusy = true;  // El módulo está ocupado intentando unirse
    _isJoined = false; // Reseteamos estado hasta confirmación
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::sendData(uint8_t port, const char* data, bool confirmed)
{
  if (!_isJoined)
    return false;
  if (_isTxBusy)
    return false; // Bloqueo de seguridad

  // Buffer temporal para el comando completo
  char cmdBuffer[AT_BUFFER_SIZE];
  int  written = snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+SEND=%d:%d:%s", port, confirmed ? 1 : 0, data);

  if (written >= sizeof(cmdBuffer))
    return false; // Overflow

  AtError err = _parser.sendCommand(cmdBuffer);

  if (err == AtError::OK) {
    _isTxBusy = true; // Marcamos ocupado hasta recibir evento TX o RX
    return true;
  }
  else if (err == AtError::BUSY) {
    // El módulo dice que está ocupado (Duty cycle o RX windows)
    _isTxBusy = true;
  }

  return false;
}

bool LSM1x0A_Controller::setKeys(const char* devEui, const char* appKey, const char* appEui)
{
  char cmd[64];
  // Configurar DevEUI
  if (devEui != nullptr) {
    snprintf(cmd, sizeof(cmd), "AT+DEUI=%s", devEui);
    if (_parser.sendCommand(cmd) != AtError::OK)
      return false;
    log(LsmLogLevel::DEBUG, "DevEUI configurado: %s", devEui);
  }

  // Configurar AppKey
  if (appKey != nullptr) {
    snprintf(cmd, sizeof(cmd), "AT+APPKEY=%s", appKey);
    if (_parser.sendCommand(cmd) != AtError::OK)
      return false;
    log(LsmLogLevel::DEBUG, "AppKey configurada: %s", appKey);
  }

  // Configurar AppEUI
  if (appEui != nullptr) {
    snprintf(cmd, sizeof(cmd), "AT+APPEUI=%s", appEui);
    if (_parser.sendCommand(cmd) != AtError::OK)
      return false;
    log(LsmLogLevel::DEBUG, "AppEUI configurada: %s", appEui);
  }
  return true;
}

bool LSM1x0A_Controller::setBand(uint8_t band)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "AT+BAND=%d", band);
  AtError err = _parser.sendCommand(cmd);
  if (err == AtError::OK) {
    log(LsmLogLevel::DEBUG, "Banda LoRaWAN configurada: %d", band);
    return true;
  }
  return false;
}

void LSM1x0A_Controller::getDevEUI(char* outBuffer, size_t outSize)
{
  AtError err = _parser.sendCommandWithResponse("AT+DEUI=?", nullptr, outBuffer, outSize);
  if (err != AtError::OK) {
    if (outSize > 0)
      outBuffer[0] = '\0'; // Cadena vacía en caso de error
  }
}

bool LSM1x0A_Controller::isJoined() const
{
  return _isJoined;
}

bool LSM1x0A_Controller::isTxBusy() const
{
  return _isTxBusy;
}