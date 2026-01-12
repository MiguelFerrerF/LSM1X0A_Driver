#include "LSM1x0A_AtParser.h"
#include <Arduino.h>

LSM1x0A_AtParser::LSM1x0A_AtParser()
{
  _syncSem = xSemaphoreCreateBinary();
}

bool LSM1x0A_AtParser::init(UartDriver* driver, AtEventCallback onEvent, void* eventCtx)
{
  _driver         = driver;
  _eventCallback  = onEvent;
  _eventCtx       = eventCtx;
  _lineIdx        = 0;
  _pendingCommand = false;

  // Initialize the UART driver with the static callback
  return _driver->init(UART_NUM_2, LSM1X0A_BAUDRATE, LSM1X0A_RX_PIN, LSM1X0A_TX_PIN, LSM1x0A_AtParser::staticRxCallback, this);
}

void LSM1x0A_AtParser::staticRxCallback(void* ctx, uint8_t* data, size_t len)
{
  // Cast seguro porque sabemos que ctx es 'this'
  ((LSM1x0A_AtParser*)ctx)->eatBuffer(data, len);
}

// Basado en lora_command.c y lora_at.h
AtError LSM1x0A_AtParser::parseErrorString(const char* line)
{
  if (strstr(line, "AT_PARAM_ERROR"))
    return AtError::PARAM_ERROR;
  if (strstr(line, "AT_BUSY_ERROR"))
    return AtError::BUSY;
  if (strstr(line, "AT_NO_NETWORK_JOINED"))
    return AtError::NO_NET_JOINED;
  if (strstr(line, "AT_RX_ERROR"))
    return AtError::RX_ERROR;
  if (strstr(line, "AT_TEST_PARAM_OVERFLOW"))
    return AtError::TEST_PARAM_OVERFLOW;
  if (strstr(line, "AT_ERROR"))
    return AtError::GENERIC_ERROR; // Error genérico fallback
  if (strstr(line, "BOOTALERT"))
    return AtError::BOOT_ALERT;
  return AtError::UNKNOWN;
}

const char* LSM1x0A_AtParser::atErrorToString(AtError err)
{
  if (err == AtError::OK)
    return "OK";
  if (err == AtError::GENERIC_ERROR)
    return "AT_ERROR";
  if (err == AtError::PARAM_ERROR)
    return "AT_PARAM_ERROR";
  if (err == AtError::BUSY)
    return "AT_BUSY_ERROR";
  if (err == AtError::TEST_PARAM_OVERFLOW)
    return "AT_TEST_PARAM_OVERFLOW";
  if (err == AtError::NO_NET_JOINED)
    return "AT_NO_NETWORK_JOINED";
  if (err == AtError::RX_ERROR)
    return "AT_RX_ERROR";
  if (err == AtError::TIMEOUT)
    return "TIMEOUT";
  if (err == AtError::BUFFER_OVERFLOW)
    return "BUFFER_OVERFLOW";
  if (err == AtError::BOOT_ALERT)
    return "BOOTALERT";
  return "UNKNOWN";
}

void LSM1x0A_AtParser::eatBuffer(const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    char c = (char)data[i];
    // Detección de fin de línea estándar \r\n
    if (c == '\n' || c == '\r') {
      if (_lineIdx > 0) {
        _lineBuffer[_lineIdx] = '\0';                                // Null-terminate
        Serial.printf("AT Parser recibió línea: %s\n", _lineBuffer); // Debug
        processLine(_lineBuffer);
        _lineIdx = 0; // Reset para siguiente línea
      }
    }
    else {
      // Protección contra desbordamiento de buffer interno
      if (_lineIdx < AT_BUFFER_SIZE - 1) {
        _lineBuffer[_lineIdx++] = c;
      }
      // Si se llena, ignoramos el resto de la línea hasta el \n
    }
  }
}

void LSM1x0A_AtParser::processLine(char* line)
{
  // 1. Detección de Respuesta a Comando Síncrono
  if (_pendingCommand) {
    // ¿Es OK? Termina transacción con éxito
    if (strcmp(line, "OK") == 0) {
      _lastResultError = AtError::OK;
      xSemaphoreGive(_syncSem);
      return;
    }

    // ¿Es un Error conocido? Termina transacción con fallo
    AtError err = parseErrorString(line);
    if (err != AtError::UNKNOWN) {
      _lastResultError = err;
      xSemaphoreGive(_syncSem);
      return;
    }

    // ¿Es el dato que esperábamos? (Ej: "DevEui: ...")
    if (_userOutBuffer != nullptr) {
      const char* dataStart = line;
      bool        match     = false;

      // Opción A: Buscamos un tag específico (Ej "DevEui:")
      if (_expectedTag != nullptr) {
        size_t tagLen = strlen(_expectedTag);
        if (strncmp(line, _expectedTag, tagLen) == 0) {
          dataStart = line + tagLen; // Avanzar puntero después del tag
          match     = true;
        }
      }
      // Opción B: No hay tag, tomamos la línea entera (salvo si es OK/ERROR que ya filtramos arriba)
      else {
        match = true;
      }

      if (match) {
        // Limpieza de espacios iniciales (trim left)
        while (*dataStart == ' ' || *dataStart == ':')
          dataStart++;

        // Copia segura al buffer del usuario
        size_t len = strlen(dataStart);
        if (len < _userOutSize) {
          strncpy(_userOutBuffer, dataStart, _userOutSize);
          _userOutBuffer[len] = '\0'; // Asegurar null-termination
        }
        else {
          // Si no cabe, copiamos lo que entre y marcamos error internamente si quisiéramos
          strncpy(_userOutBuffer, dataStart, _userOutSize - 1);
          _userOutBuffer[_userOutSize - 1] = '\0';
        }
        // No liberamos semáforo todavía, esperamos el "OK" final
      }
    }
  }

  // 2. Detección de Eventos Asíncronos (URC)
  // Lógica desacoplada: Siempre vigilando
  if (_eventCallback) {
    if (strncmp(line, "+EVT", 3) == 0 || strncmp(line, "RX", 2) == 0) {
      // Pasamos el puntero al resto de la línea como payload
      const char* payload = strchr(line, ':');
      _eventCallback("RX", payload ? payload + 1 : line, _eventCtx);
    }
    else if (strstr(line, "JOINED")) {
      _eventCallback("JOIN", "SUCCESS", _eventCtx);
    }
    // Puedes añadir más eventos aquí mirando lora_at.c
  }
}

AtError LSM1x0A_AtParser::sendCommand(const char* cmd, uint32_t timeoutMs)
{
  return sendCommandWithResponse(cmd, nullptr, nullptr, 0, timeoutMs);
}

AtError LSM1x0A_AtParser::sendCommandWithResponse(const char* cmd, const char* expectedTag, char* outBuffer, size_t outSize, uint32_t timeoutMs)
{
  // 1. Asegurar que no hay otra transacción en curso
  xSemaphoreTake(_syncSem, 0);

  if (_pendingCommand) {
    return AtError::BUSY;
  }

  // 2. Configurar contexto de la transacción
  _pendingCommand  = true;
  _lastResultError = AtError::TIMEOUT;
  _expectedTag     = expectedTag;
  _userOutBuffer   = outBuffer; // Puntero al stack/global del usuario
  _userOutSize     = outSize;

  // Limpiar buffer de usuario por seguridad
  if (outBuffer)
    outBuffer[0] = '\0';

  // 3. Enviar Comando por UART
  _driver->sendData(cmd, strlen(cmd));

  // 4. Bloquear esperando respuesta
  if (xSemaphoreTake(_syncSem, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
    _pendingCommand = false;
    return AtError::TIMEOUT;
  }

  _pendingCommand = false;
  _userOutBuffer  = nullptr; // Desvincular buffer por seguridad

  return _lastResultError;
}