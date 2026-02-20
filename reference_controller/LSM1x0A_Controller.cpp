#include "LSM1x0A_Controller.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "hal/gpio_types.h"
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------
// CONSTRUCTOR
// ---------------------------------------------------------
LSM1x0A_Controller::LSM1x0A_Controller()
{

  _isJoined                = false;
  _shadowConfig.isLoRaMode = true;
  _userCallback            = nullptr;
  _currentLogLevel         = LsmLogLevel::INFO; // Default: Solo información útil
  _maxTimeoutErrors        = 3;
  _timeoutCounter          = 0;

  _safetyTimer = xTimerCreate("LSM_Safety", pdMS_TO_TICKS(10000), pdFALSE, (void*)this, safetyTimerCallback);
  _stateMutex  = xSemaphoreCreateMutex();
  setTxBusy(false);
}

LSM1x0A_Controller::~LSM1x0A_Controller()
{
  if (_safetyTimer) {
    if (xTimerIsTimerActive(_safetyTimer)) {
      xTimerStop(_safetyTimer, 0);
    }
    xTimerDelete(_safetyTimer, 0);
    _safetyTimer = nullptr;
  }

  if (_stateMutex) {
    vSemaphoreDelete(_stateMutex);
    _stateMutex = nullptr;
  }
}
// ---------------------------------------------------------
// SISTEMA DE LOGGING INTERNO
// ---------------------------------------------------------
void LSM1x0A_Controller::setLogLevel(LsmLogLevel level)
{
  _currentLogLevel = level;
}

void LSM1x0A_Controller::log(LsmLogLevel level, const char* format, ...)
{
  // 1. Filtrado Eficiente: Si el nivel del mensaje es superior al configurado, ignorar.
  if (level > _currentLogLevel)
    return;
  if (!_userCallback)
    return;

  // 2. Formateo de String (estilo printf)
  char    buffer[256]; // Buffer seguro para logs
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

  log(LsmLogLevel::ERROR, "RECOVERY: Ejecutando HARD RESET (GPIO %d)...", _resetPin);

  gpio_reset_pin((gpio_num_t)_resetPin);
  gpio_set_direction((gpio_num_t)_resetPin, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)_resetPin, 0); // Bajar a GND
  vTaskDelay(pdMS_TO_TICKS(200));
  gpio_set_level((gpio_num_t)_resetPin, 1); // Subir a VCC
  vTaskDelay(pdMS_TO_TICKS(1000));
  return true;
}

bool LSM1x0A_Controller::softReset()
{
  log(LsmLogLevel::ERROR, "RECOVERY: Ejecutando SOFT RESET...");
  char    moduleType[20] = {0};
  AtError res            = _parser.sendCommandWithResponse(LsmAtCommand::RESET, "Device : ", moduleType, sizeof(moduleType), 5000);
  if (res != AtError::BOOT_ALERT) {
    log(LsmLogLevel::ERROR, "Fallo en SOFT RESET: El módulo no respondió correctamente (Error: %s)", _parser.atErrorToString(res));
    return false;
  }
  log(LsmLogLevel::INFO, "SOFT RESET exitoso, módulo reiniciado.");
  _isLSM110A = (strstr(moduleType, "LSM110A") != nullptr);
  if (_isLSM110A)
    log(LsmLogLevel::INFO, "Módulo identificado como LSM110A.");
  else
    log(LsmLogLevel::INFO, "Módulo identificado como LSM100A.");
  return true;
}

// ---------------------------------------------------------
// SISTEMA DE RECUPERACIÓN AUTOMÁTICA
// ---------------------------------------------------------
void LSM1x0A_Controller::setMaxTimeoutErrors(uint8_t maxErrors)
{
  _maxTimeoutErrors = maxErrors;
}

bool LSM1x0A_Controller::recoverModule()
{
  log(LsmLogLevel::ERROR, "RECOVERY: Iniciando proceso de recuperación automática...");

  bool recovered = false;

  if (softReset())
    recovered = true;
  else if (hardReset())
    if (softReset())
      recovered = true;

  if (!recovered) {
    log(LsmLogLevel::ERROR, "RECOVERY: No se pudo reiniciar el módulo.");
    return false;
  }

  // IMPORTANTE: Al restablecer el módulo pierde la conexión de sesión
  _isJoined = false;

  log(LsmLogLevel::INFO, "RECOVERY: Módulo reiniciado pidiendo reconexión. Restaurando configuración...");

  if (!restoreConfiguration()) {
    log(LsmLogLevel::ERROR, "RECOVERY: Fallo al restaurar configuración.");
    return false;
  }

  log(LsmLogLevel::INFO, "RECOVERY: Configuración restaurada. Sistema listo.");
  return true;
}

bool LSM1x0A_Controller::restoreConfiguration()
{
  // Restaurar Modo
  if (!setMode(_shadowConfig.isLoRaMode))
    return false;

  // Restaurar configuración volátil de LoRaWAN
  if (_shadowConfig.isLoRaMode) {
    if (strlen(_shadowConfig.devEui) > 0 && !setDevEUI(_shadowConfig.devEui))
      return false;
    if (strlen(_shadowConfig.devAddr) > 0 && !setDevAddr(_shadowConfig.devAddr))
      return false;
    if (strlen(_shadowConfig.nwkID) > 0 && !setNwkID(_shadowConfig.nwkID))
      return false;

    if (!setBand(_shadowConfig.band))
      return false;
    if (!setADR(_shadowConfig.adrEnabled))
      return false;
    if (!_shadowConfig.adrEnabled && !setDataRate(_shadowConfig.dr))
      return false;
    if (!setTxPower(_shadowConfig.txPower))
      return false;

    if (!setDutyCycle(_shadowConfig.dutyCycle))
      return false;
    if (!setJoin1Delay(_shadowConfig.join1Delay))
      return false;
    if (!setJoin2Delay(_shadowConfig.join2Delay))
      return false;
    if (!setRx1Delay(_shadowConfig.rx1Delay))
      return false;
    if (!setRx2Delay(_shadowConfig.rx2Delay))
      return false;
    if (!setRx2DataRate(_shadowConfig.rx2DataRate))
      return false;
    if (!setRx2Frequency(_shadowConfig.rx2Frequency))
      return false;
    // if (!setChannelMaskBySubBand(_shadowConfig.band, _shadowConfig.subBand))
  }

  // Restaurar configuración volátil de Sigfox
  if (!_shadowConfig.isLoRaMode) {
    if (!setRCChannel(_shadowConfig.rcChannel))
      return false;
    if (!setRadioPower(_shadowConfig.radioPower))
      return false;
    if (!setEncryptKey(_shadowConfig.encryptKey))
      return false;
    if (!setEncryptPayload(_shadowConfig.encryptPayload))
      return false;
  }

  return true;
}

bool LSM1x0A_Controller::readActualConfiguration()
{
  _shadowConfig.isLoRaMode = getMode();
  if (_shadowConfig.isLoRaMode) {
    getDevEUI(_shadowConfig.devEui, sizeof(_shadowConfig.devEui));
    getDevAddr(_shadowConfig.devAddr, sizeof(_shadowConfig.devAddr));
    getNwkID(_shadowConfig.nwkID, sizeof(_shadowConfig.nwkID));
    _shadowConfig.adrEnabled = getADR();
    if (!_shadowConfig.adrEnabled)
      _shadowConfig.dr = getDataRate();
    _shadowConfig.txPower        = getTxPower();
    _shadowConfig.band           = getBand();
    _shadowConfig.dutyCycle      = getDutyCycle();
    _shadowConfig.join1Delay     = getJoin1Delay();
    _shadowConfig.join2Delay     = getJoin2Delay();
    _shadowConfig.rx1Delay       = getRx1Delay();
    _shadowConfig.rx2Delay       = getRx2Delay();
    _shadowConfig.rx2DataRate    = getRx2DataRate();
    _shadowConfig.rx2Frequency   = getRx2Frequency();
    _shadowConfig.confirmRetry   = getConfirmRetry();
    _shadowConfig.unconfirmRetry = getUnconfirmedRetry();
  }
  else {
    _shadowConfig.rcChannel      = getRCChannel();
    _shadowConfig.radioPower     = getRadioPower();
    _shadowConfig.encryptKey     = getEncryptKey();
    _shadowConfig.encryptPayload = getEncryptPayload();
  }
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
    log(LsmLogLevel::LSM_DEBUG, "Parser y Driver UART iniciados correctamente.");

  log(LsmLogLevel::LSM_DEBUG, "Sincronizando UART...");
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
    log(LsmLogLevel::LSM_DEBUG, "Sincronización exitosa tras reset.");
  }
  log(LsmLogLevel::INFO, "Controlador LSM1x0A iniciado correctamente.");

  // Leemos la configuración actual del módulo para tener un estado inicial correcto
  readActualConfiguration();

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
  // --- VERBOSE LOGGING ---
  if (strcmp(type, LsmEvent::VERBOSE) == 0) {
    log(LsmLogLevel::VERBOSE, ">> %s", payload);
  }

  bool operationFinished = false;

  // --- JOIN ---
  if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      log(LsmLogLevel::INFO, ">>> APP: Unión exitosa");
      _isJoined = true;
    }
    else {
      log(LsmLogLevel::ERROR, ">>> APP: Fallo al unir. (Ver logs para motivo)");
      _isJoined = false;
    }
    operationFinished = true;
  }
  else if (strcmp(type, LsmEvent::TX) == 0 || strcmp(type, LsmEvent::RX_DATA) == 0 || strcmp(type, LsmEvent::RX_META) == 0) {
    _isJoined         = true;
    operationFinished = true;
  }

  // GESTIÓN SEGURA DEL TIMER Y ESTADO
  if (operationFinished) {
    if (_stateMutex) {
      xSemaphoreTake(_stateMutex, portMAX_DELAY);
      if (_safetyTimer) {
        xTimerStop(_safetyTimer, 0);
      }
      _isTxBusy = false;
      xSemaphoreGive(_stateMutex);
    }
  }

  if (_userCallback) {
    _userCallback(type, payload);
  }
}

// ---------------------------------------------------------
// PROCESO DE JOIN
// ---------------------------------------------------------

bool LSM1x0A_Controller::lora_join(bool isOTAA)
{
  if (!_shadowConfig.isLoRaMode)
    return false;
  if (isTxBusy())
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN, isOTAA ? 1 : 0);
  startSafetyTimer(20000);
  if (executeCommandWithRetries(cmd) == AtError::OK) {
    _isJoined = false; // Reseteamos estado hasta confirmación
    log(LsmLogLevel::INFO, "LoRaWAN: Iniciando proceso de 'Join' usando %s...", isOTAA ? "OTAA" : "ABP");
    setTxBusy(true);
    return true;
  }
  return false;
}

bool LSM1x0A_Controller::sigfox_join()
{
  if (_shadowConfig.isLoRaMode)
    return false;
  if (isTxBusy())
    return false;

  log(LsmLogLevel::INFO, "Sigfox: Iniciando 'Join' (Test de cobertura bidireccional)...");
  log(LsmLogLevel::INFO, "Sigfox: Esto tardara aprox 60 segundos. Espere...");

  char rxBuffer[32] = {0};

  // 2. Envío usando el comando específico de Join
  // Nota: Usamos timeout largo (60s)
  AtError err     = executeCommandWithRetries(LsmAtCommand::SEND_BIT_CONFIRMED, "+RX_H=", rxBuffer, sizeof(rxBuffer), 60000);
  bool    success = handleSigfoxDownlinkResponse(err, rxBuffer);
  if (success)
    _isJoined = true;
  else
    _isJoined = false;

  if (_userCallback)
    _userCallback(LsmEvent::JOIN, success ? "SUCCESS" : "FAILED");
  return success;
}

// ---------------------------------------------------------
// ENVÍO DE DATOS
// ---------------------------------------------------------

bool LSM1x0A_Controller::lora_sendData(uint8_t port, const char* data, bool confirmed)
{
  if (!_shadowConfig.isLoRaMode)
    return false;
  if (!_isJoined)
    return false;
  if (isTxBusy())
    return false;

  // Buffer temporal para el comando completo
  char cmdBuffer[AT_LORA_BUFFER_SIZE];
  int  written = snprintf(cmdBuffer, sizeof(cmdBuffer), "%s%d:%d:%s", LsmAtCommand::SEND, port, confirmed ? 1 : 0, data);

  if (written >= sizeof(cmdBuffer)) {
    log(LsmLogLevel::ERROR, "LoRaWAN: Payload demasiado grande para el buffer de comando (tamaño máximo: %d)", sizeof(cmdBuffer) - 1);
    if (_userCallback)
      _userCallback(LsmEvent::TX, "FAILED");
    return false;
  }

  AtError err = executeCommandWithRetries(cmdBuffer);

  uint32_t preciseTimeout = 0;
  if (confirmed)
    preciseTimeout = calculateBlockingTime(_shadowConfig.dr, _shadowConfig.confirmRetry);
  else
    preciseTimeout = calculateBlockingTime(_shadowConfig.dr, _shadowConfig.unconfirmRetry);
  startSafetyTimer(preciseTimeout);
  log(LsmLogLevel::INFO, "TX LoRa: DR%d. Watchdog ajustado a %lu ms", static_cast<int>(_shadowConfig.dr), preciseTimeout);

  if (err == AtError::OK) {
    setTxBusy(true);

    return true;
  }
  else if (err == AtError::BUSY) {
    setTxBusy(true);
    log(LsmLogLevel::ERROR, "LoRaWAN: Módulo ocupado, no se puede transmitir ahora.");
    if (_userCallback)
      _userCallback(LsmEvent::TX, "BUSY");
    startSafetyTimer(30000);
  }
  else if (err == AtError::NO_NET_JOINED) {
    _isJoined = false; // El módulo no está unido a la red
    setTxBusy(false);
    log(LsmLogLevel::ERROR, "LoRaWAN: No unido a la red, no se puede transmitir.");
    if (_userCallback)
      _userCallback(LsmEvent::TX, "NOT JOINED");
  }
  else if (err == AtError::DUTY_CYCLE_RESTRICT) {
    setTxBusy(false);
    log(LsmLogLevel::ERROR, "LoRaWAN: Restricción de duty cycle, no se puede transmitir ahora.");
    startSafetyTimer(60000); // Esperar 60s antes de permitir otro intento
  }
  else {
    setTxBusy(false);
    log(LsmLogLevel::ERROR, "LoRaWAN: Error en envío AT (Err: %s)", _parser.atErrorToString(err));
    if (_userCallback)
      _userCallback(LsmEvent::TX, "FAILED");
  }
  return false;
}

bool LSM1x0A_Controller::sigfox_sendData(const char* data, LsmSigfoxDataType type, bool confirmed, uint8_t retry)
{
  if (!canTransmit(false)) {
    log(LsmLogLevel::ERROR, "Sigfox: No se puede transmitir en este momento (Modo: %s, Unido: %s, Ocupado: %s)",
        _shadowConfig.isLoRaMode ? "LoRa" : "Sigfox", _isJoined ? "Sí" : "No", isTxBusy() ? "Sí" : "No");
    return false;
  }

  bool dataIsValid = true;

  if (type == LsmSigfoxDataType::BIT && (strcmp(data, "0") != 0 && strcmp(data, "1") != 0)) {
    log(LsmLogLevel::ERROR, "Sigfox Validation: Data BIT inválido '%s', debe ser '0' o '1'.", data);
    dataIsValid = false;
  }
  else if (type == LsmSigfoxDataType::ASCII_DATA) {
    if (!isValidAscii(data, 24)) {
      log(LsmLogLevel::ERROR, "Sigfox Validation: Data ASCII contiene caracteres no imprimibles o excede 24 chars.");
      dataIsValid = false;
    }
    else if (strlen(data) > 12) {
      // El módulo soporta hasta 12 bytes en frame ASCII
      log(LsmLogLevel::ERROR, "Sigfox Validation: Data ASCII excede los 12 caracteres máximos.");
      dataIsValid = false;
    }
  }
  else if (type == LsmSigfoxDataType::HEX_DATA) {
    if (!isValidHex(data, 24)) {
       log(LsmLogLevel::ERROR, "Sigfox Validation: Data HEX contiene caracteres erróneos.");
       dataIsValid = false;
    }
    else if (strlen(data) > 24) { 
      // Hasta 12 bytes de payload = 24 caracteres hex
      log(LsmLogLevel::ERROR, "Sigfox Validation: Data HEX excede 24 caracteres.");
      dataIsValid = false;
    }
  }

  // Si la validación falla, notificamos evento FAILED limpio y salimos
  if (!dataIsValid) {
    if (_userCallback)
      _userCallback(LsmEvent::TX, "FAILED");
    return false;
  }

  // Log informativo (OOB ignora datos)
  if (type == LsmSigfoxDataType::OOB && (strlen(data) > 0 || confirmed)) {
    log(LsmLogLevel::INFO, "Sigfox: Data OOB activado (parámetros extra ignorados).");
  }

  // Buffer temporal para el comando completo
  char        cmdBuffer[AT_SIGFOX_BUFFER_SIZE];
  const char* atCmd   = "";
  int         ackFlag = confirmed ? 1 : 0;

  switch (type) {
    case LsmSigfoxDataType::BIT:
      atCmd = LsmAtCommand::SEND_BIT;
      break;
    case LsmSigfoxDataType::ASCII_DATA:
      atCmd = LsmAtCommand::SEND_FRAME;
      break;
    case LsmSigfoxDataType::HEX_DATA:
      atCmd = LsmAtCommand::SEND_HEX;
      break;
    case LsmSigfoxDataType::OOB:
      atCmd = LsmAtCommand::SEND_OOB;
      break;
    default:
      return false;
  }

  if (type == LsmSigfoxDataType::OOB)
    snprintf(cmdBuffer, sizeof(cmdBuffer), "%s", atCmd);
  else if (type == LsmSigfoxDataType::HEX_DATA)
    snprintf(cmdBuffer, sizeof(cmdBuffer), "%s%d,%s,%d,%d", atCmd, (int)strlen(data), data, ackFlag, retry);
  else
    snprintf(cmdBuffer, sizeof(cmdBuffer), "%s%s,%d,%d", atCmd, data, ackFlag, retry);

  // Envío confirmado
  if (confirmed) {
    char    rxBuffer[32] = {0};
    AtError err          = executeCommandWithRetries(cmdBuffer, "+RX_H=", rxBuffer, sizeof(rxBuffer), 60000);
    bool    success      = handleSigfoxDownlinkResponse(err, rxBuffer);
    if (success) {
      setTxBusy(false);
      if (_userCallback)
        _userCallback(LsmEvent::TX, "SUCCESS");
    }
    else {
      if (_userCallback)
        _userCallback(LsmEvent::TX, "FAILED");
    }
    return success;
  }

  // Envío NO Confirmado
  AtError err = executeCommandWithRetries(cmdBuffer, nullptr, nullptr, 0, 30000);
  if (err != AtError::OK) {
    log(LsmLogLevel::ERROR, "Sigfox: Error en envío AT (Err: %s)", _parser.atErrorToString(err));
    if (_userCallback)
      _userCallback(LsmEvent::TX, "FAILED");
    return false;
  }

  // Éxito
  setTxBusy(false);
  if (_userCallback)
    _userCallback(LsmEvent::TX, "SUCCESS");
  return true;
}

// ---------------------------------------------------------
// RECUPERACIÓN DE ESTADO
// ---------------------------------------------------------
bool LSM1x0A_Controller::isJoined() const
{
  return _isJoined;
}

bool LSM1x0A_Controller::isTxBusy() const
{
  bool state = false;
  if (_stateMutex) {
    xSemaphoreTake(_stateMutex, portMAX_DELAY);
    state = _isTxBusy;
    xSemaphoreGive(_stateMutex);
  }
  return state;
}

// ---------------------------------------------------------
// SLEEP & POWER MANAGEMENT
// ---------------------------------------------------------
bool LSM1x0A_Controller::sleep()
{
  if (isTxBusy()) {
    log(LsmLogLevel::ERROR, "No se puede entrar en SLEEP: Módulo ocupado transmitiendo.");
    return false;
  }

  gpio_set_level((gpio_num_t)LSM1X0A_TX_PIN, 0); // Aseguramos que TX esté en bajo para evitar ruido
  gpio_set_direction((gpio_num_t)LSM1X0A_TX_PIN, GPIO_MODE_OUTPUT);

  log(LsmLogLevel::INFO, "Modo Sleep activado. UART Deshabilitado.");
  return true;
}

bool LSM1x0A_Controller::wakeup()
{
  gpio_set_direction((gpio_num_t)LSM1X0A_TX_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)LSM1X0A_TX_PIN, 1); // Subimos a VCC para reactivar UART
  vTaskDelay(pdMS_TO_TICKS(50));                 // Pequeña espera para estabilizar la UART
  log(LsmLogLevel::INFO, "Modo Sleep desactivado.");
  return true;
}

// ---------------------------------------------------------
// HELPERS INTERNOS
// ---------------------------------------------------------
bool LSM1x0A_Controller::canTransmit(bool requireLoRaMode)
{
  if (_shadowConfig.isLoRaMode != requireLoRaMode)
    return false;
  if (!_isJoined && requireLoRaMode) // Si es LoRa, debe estar unido, si es Sigfox no hace falta
    return false;
  if (isTxBusy())
    return false;
  return true;
}

void LSM1x0A_Controller::parseTimestamp(const char* rxBuffer, char* outBuffer)
{
  if (rxBuffer == nullptr || strlen(rxBuffer) < 8) {
    log(LsmLogLevel::ERROR, "parseTimestamp: Buffer nulo o demasiado pequeño.");
    if (outBuffer) outBuffer[0] = '\0';
    return;
  }

  char hexTemp[9];
  // Parsear TIMESTAMP (Primeros 8 caracteres)
  memcpy(hexTemp, rxBuffer, 8);
  hexTemp[8]  = '\0';
  uint32_t ts = strtoul(hexTemp, NULL, 16);

  // Ajustar el reloj interno del ESP32 con esta hora
  time_t         rawTime = (time_t)ts;
  struct timeval tv      = {.tv_sec = static_cast<long>(rawTime), .tv_usec = 0};
  settimeofday(&tv, NULL);

  // Convertir a string legible
  struct tm* timeInfo;
  timeInfo = gmtime(&rawTime);
  if (timeInfo && outBuffer) {
    strftime(outBuffer, 32, "%Y-%m-%d %H:%M:%S", timeInfo);
  }
}

void LSM1x0A_Controller::parseRSSI(const char* rxBuffer)
{
  char hexTemp[5];
  // Parsear RSSI (Siguientes 4 caracteres)
  memcpy(hexTemp, rxBuffer + 8, 4);
  hexTemp[4]       = '\0';
  uint32_t rawRssi = strtoul(hexTemp, NULL, 16); // RSSI en formato sin signo
  _currentRSSI     = (int16_t)rawRssi;           // Convertir a signed
}

bool LSM1x0A_Controller::handleSigfoxDownlinkResponse(AtError err, const char* rxBuffer)
{
  if (err != AtError::OK) {
    log(LsmLogLevel::ERROR, "Sigfox: Error o Sin Respuesta (Err: %s)", _parser.atErrorToString(err));
    return false;
  }

  if (strlen(rxBuffer) >= 12) { // Mínimo 12 caracteres esperados (TIMESTAMP + RSSI)
    char dataString[32];
    parseTimestamp(rxBuffer, dataString);
    parseRSSI(rxBuffer);

    log(LsmLogLevel::INFO, "Sigfox ACK: Fecha %s, RSSI %d dBm", dataString, _currentRSSI);

    // Notificar metadatos RX al usuario
    char metaPayload[64];
    snprintf(metaPayload, sizeof(metaPayload), "RX_SF, RSSI %d, SNR 0", _currentRSSI);

    if (_userCallback)
      _userCallback(LsmEvent::RX_META, metaPayload);

    return true;
  }

  log(LsmLogLevel::ERROR, "Sigfox Downlink: Formato de respuesta incorrecto.");
  return false;
}

bool LSM1x0A_Controller::isValidHex(const char* str, size_t maxLen)
{
  if (str == nullptr) {
    log(LsmLogLevel::ERROR, "Validation Error: Cadena nula.");
    return false;
  }

  size_t length = 0;
  while (*str && length < maxLen) {
    if (!((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F') || (*str >= 'a' && *str <= 'f'))) {
      return false; // Caracter no-hexadecimal detectado
    }
    str++;
    length++;
  }

  if (length >= maxLen) {
    log(LsmLogLevel::ERROR, "Validation Warning: Longitud máxima superada.");
    return false;
  }
  return true;
}

bool LSM1x0A_Controller::isValidAscii(const char* str, size_t maxLen)
{
  if (str == nullptr) {
    log(LsmLogLevel::ERROR, "Validation Error: Cadena nula.");
    return false;
  }

  size_t length = 0;
  while (*str && length < maxLen) {
    // Aceptamos rango de caracteres imprimibles ASCII (incluyendo espacio)
    if (*str < 32 || *str > 126) {
      return false; // Caracter de control o no estándar detectado
    }
    str++;
    length++;
  }

  if (length >= maxLen) {
    log(LsmLogLevel::ERROR, "Validation Warning: Longitud máxima superada.");
    return false;
  }
  return true;
}

AtError LSM1x0A_Controller::executeCommandWithRetries(const char* cmd, const char* expectedTag, char* outBuffer, size_t outSize, uint32_t timeoutMs)
{
  AtError err;
  for (uint8_t attempt = 0; attempt < _maxTimeoutErrors; attempt++) {
    if (outBuffer != nullptr)
      err = _parser.sendCommandWithResponse(cmd, expectedTag, outBuffer, outSize, timeoutMs);
    else
      err = _parser.sendCommand(cmd, timeoutMs);

    if (err == AtError::OK) {
      return AtError::OK;
    }
    else if (err == AtError::BUSY) {
      log(LsmLogLevel::ERROR, "Comando '%s' fallido: Módulo ocupado (BUSY).", cmd);
      return err;
    }
    else if (err == AtError::TIMEOUT) {
      _timeoutCounter++;
      if (_timeoutCounter > _maxTimeoutErrors) {
        log(LsmLogLevel::ERROR, "Comando '%s' ha fallado %d veces por TIMEOUT, intentando recuperación...", cmd, _timeoutCounter);
        if (recoverModule()) {
          log(LsmLogLevel::INFO, "Recuperación exitosa tras TIMEOUTs en comando '%s'. Reintentando comando...", cmd);
          _timeoutCounter = 0; // Resetear contador tras recuperación
          // Avisar al cliente que necesita unirse de nuevo si es LoRaWAN
          if (_shadowConfig.isLoRaMode && _userCallback) {
            _userCallback(LsmEvent::JOIN, "REQUIRED");
          }
        }
        else {
          log(LsmLogLevel::ERROR, "Fallo crítico: No se pudo recuperar módulo tras TIMEOUTs en comando '%s'.", cmd);
          return AtError::UNKNOWN;
        }
      }
    }
    else if (err == AtError::GENERIC_ERROR) {
      log(LsmLogLevel::ERROR, "Comando '%s' fallido: Error genérico (GENERIC_ERROR). Verifique los valores enviados.", cmd);
      setTxBusy(false);
      return err;
    }
    else {
      // Si falló (y no fue timeout, o fue timeout pero abajo lo manejamos),
      log(LsmLogLevel::ERROR, "Comando '%s' fallido (Error: %s), reintentando... (%d/%d)", cmd, _parser.atErrorToString(err), attempt + 1,
          _maxTimeoutErrors);
      vTaskDelay(pdMS_TO_TICKS(200));
      // No devolvemos false aquí, dejamos que el bucle continúe reintentando
    }
  }
  log(LsmLogLevel::ERROR, "Comando '%s' fallido tras %d intentos.", cmd, _maxTimeoutErrors);
  return err;
}

// ---------------------------------------------------------
// SAFETY TIMER & MATH HELPERS
// ---------------------------------------------------------

void LSM1x0A_Controller::startSafetyTimer(uint32_t durationMs)
{
  if (_stateMutex) {
    xSemaphoreTake(_stateMutex, portMAX_DELAY);
    if (_safetyTimer == nullptr) {
      // Crear si no existe
      _safetyTimer = xTimerCreate("LSM_Safety", pdMS_TO_TICKS(durationMs), pdFALSE, (void*)this, safetyTimerCallback);
      if (_safetyTimer == nullptr) {
        log(LsmLogLevel::ERROR, "No se pudo crear SafetyTimer (Out of Memory)");
        xSemaphoreGive(_stateMutex);
        return;
      }
    }

    // xTimerChangePeriod inicia el timer automáticamente si está parado.
    // El segundo parámetro (0) es el tiempo de espera si la cola de comandos del timer está llena.
    if (xTimerChangePeriod(_safetyTimer, pdMS_TO_TICKS(durationMs), 0) != pdPASS) {
      log(LsmLogLevel::ERROR, "No se pudo iniciar SafetyTimer");
    }
    xSemaphoreGive(_stateMutex);
  }
}

void LSM1x0A_Controller::safetyTimerCallback(TimerHandle_t xTimer)
{
  // Recuperamos la instancia de la clase guardada en el ID del timer
  LSM1x0A_Controller* self = (LSM1x0A_Controller*)pvTimerGetTimerID(xTimer);

  // Llamamos al método real de la instancia
  if (self) {
    self->handleSafetyTimeout();
  }
}

void LSM1x0A_Controller::handleSafetyTimeout()
{
  bool wasBusy = false;
  if (_stateMutex) {
    xSemaphoreTake(_stateMutex, portMAX_DELAY);
    if (_isTxBusy) {
      wasBusy = true;
      _isTxBusy = false;
    }
    xSemaphoreGive(_stateMutex);
  }

  // Solo actuamos si realmente pensamos que estamos ocupados
  if (wasBusy) {
    if (_userCallback) {
      _userCallback(LsmEvent::TX, "TIMEOUT");
    }
  }
}

uint32_t LSM1x0A_Controller::calculateBlockingTime(LsmDataRate dr, uint8_t totalTransmissions)
{
  // 1. Obtener ToA base
  uint16_t toa     = 0;
  int      drIndex = (int)dr;

  if (drIndex >= 0 && drIndex < (sizeof(LSM_TOA_MS) / sizeof(LSM_TOA_MS[0]))) {
    toa = LSM_TOA_MS[drIndex];
  }
  else {
    // Fallback para DR desconocidos (FSK o SF7B), usamos el peor caso medido (DR0)
    toa = 1304;
  }

  // 2. Obtener Ventana RX más tardía (RX2 Delay)
  // Usamos la configuración actual del módulo. Si no está seteada, el default LoRaWAN es 2000ms.
  // IMPORTANTE: Tus pruebas indican que usaste delays mayores.
  uint32_t rxWindow = (_shadowConfig.rx2Delay > 0) ? _shadowConfig.rx2Delay : 2000;

  // 3. Aplicar fórmula: T_bloq = ToA + RX2 + Overhead
  uint32_t t_single = toa + rxWindow + LSM_INTERNAL_PROCESS_OVERHEAD;

  // 4. Multiplicar por N (Transmisiones Totales)
  uint32_t t_total = t_single * totalTransmissions;

  // 5. Añadimos un pequeño "Epsilon" de seguridad (ej. 5s) para cubrir latencia UART/RTOS
  return t_total + 5000;
}

void LSM1x0A_Controller::setTxBusy(bool busy)
{
  if (_stateMutex) {
    xSemaphoreTake(_stateMutex, portMAX_DELAY);
    _isTxBusy = busy;
    xSemaphoreGive(_stateMutex);
  }
}