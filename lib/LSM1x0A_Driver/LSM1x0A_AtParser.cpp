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
  // 1. LIMPIEZA
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n'))
    line[--len] = '\0';
  if (len == 0)
    return;

  // 2. DETECCIÓN DE EVENTOS ASÍNCRONOS (+EVT)
  // Basado en las funciones: AT_event_join, AT_event_receive, AT_event_confirm
  if (strncmp(line, "+EVT:", 5) == 0) {
    char* payload = line + 5; // Saltamos el prefijo "+EVT:"

    if (_eventCallback) {
      // CASO A: JOIN (+EVT:JOINED o +EVT:JOIN FAILED)
      if (strncmp(payload, "JOINED", 6) == 0) {
        _eventCallback(LsmEvent::JOIN, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "JOIN FAILED", 11) == 0) {
        _eventCallback(LsmEvent::JOIN, "FAILED", _eventCtx);
      }

      // CASO B: TX CONFIRMATION (+EVT:SEND_CONFIRMED o +EVT:SEND_FAILED)
      else if (strncmp(payload, "SEND_CONFIRMED", 14) == 0) {
        _eventCallback(LsmEvent::TX, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "SEND_FAILED", 11) == 0) {
        _eventCallback(LsmEvent::TX, "FAILED", _eventCtx);
      }

      // CASO C: RX DATA (+EVT:RECV_CONFIRMED:2:04:AABB...)
      // Formato esperado: RECV_XXX:<PORT>:<SIZE>:<DATA>
      else if (strncmp(payload, "RECV_", 5) == 0) {
        // Buscamos el primer ':' después de RECV_XXX
        char* ptr = strchr(payload, ':');
        if (ptr) {
          // ptr apunta a ":2:04:AABB..."
          // Pasamos al usuario el resto de la cadena.
          // Podríamos limpiar el SIZE (04) si es redundante,
          // pero pasarlo crudo permite validación.
          // Payload al usuario: "2:04:AABB..."
          _eventCallback(LsmEvent::RX, ptr + 1, _eventCtx);
        }
      }

      // CASO D: RX METADATA (+EVT:RX_1, PORT 2, DR 5...)
      // Información extra del slot y calidad de señal
      else if (strncmp(payload, "RX_", 3) == 0) {
        _eventCallback(LsmEvent::INFO, payload, _eventCtx);
      }
    }
    return; // Evento procesado, no seguimos buscando respuestas de comandos
  }

  // 3. FILTRADO DE RUIDO DE DEBUG
  // Tu función AT_event_receive imprime "confirmed flag: X" sin +EVT. Lo ignoramos.
  if (strstr(line, "confirmed flag:"))
    return;
  if (strncmp(line, "---", 3) == 0)
    return; // Logs de boot

  // 4. MAQUINA DE ESTADOS (Respuestas a Comandos Síncronos AT)
  // Detección de BOOT
  if (strstr(line, "BOOTALERT")) {
    if (_eventCallback)
      _eventCallback("SYS", "BOOT", _eventCtx);
    if (_pendingCommand) {
      _lastResultError = AtError::BOOT_ALERT;
      xSemaphoreGive(_syncSem);
    }
    return;
  }

  if (_pendingCommand) {
    if (strcmp(line, "OK") == 0) {
      _lastResultError = AtError::OK;
      xSemaphoreGive(_syncSem);
      return;
    }

    AtError err = parseErrorString(line);
    if (err != AtError::UNKNOWN) {
      _lastResultError = err;
      xSemaphoreGive(_syncSem);
      return;
    }

    // Captura de datos (Getters)
    if (_userOutBuffer != nullptr) {
      // Lógica de extracción de valor (igual que la versión anterior)
      const char* dataStart = line;
      bool        match     = (_expectedTag == nullptr);

      if (_expectedTag && strstr(line, _expectedTag)) {
        dataStart = strstr(line, _expectedTag) + strlen(_expectedTag);
        match     = true;
      }

      if (match) {
        while (*dataStart == ' ' || *dataStart == ':')
          dataStart++;
        strncpy(_userOutBuffer, dataStart, _userOutSize - 1);
        _userOutBuffer[_userOutSize - 1] = '\0';
      }
    }
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