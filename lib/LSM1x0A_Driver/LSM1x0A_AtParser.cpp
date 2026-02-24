#include "LSM1x0A_AtParser.h"
#include <Arduino.h>

LSM1x0A_AtParser::LSM1x0A_AtParser()
{
  _syncSem = xSemaphoreCreateBinary();
}

LSM1x0A_AtParser::~LSM1x0A_AtParser()
{
  if (_syncSem != nullptr) {
    vSemaphoreDelete(_syncSem);
    _syncSem = nullptr;
  }
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
  // Usamos stricmp/strcmp en lugar de strstr para evitar falsos positivos
  if (strcmp(line, "OK") == 0)
    return AtError::OK;
  if (strcmp(line, "AT_ERROR") == 0)
    return AtError::GENERIC_ERROR;
  if (strcmp(line, "AT_PARAM_ERROR") == 0)
    return AtError::PARAM_ERROR;
  if (strcmp(line, "AT_BUSY_ERROR") == 0)
    return AtError::BUSY;
  if (strcmp(line, "AT_TEST_PARAM_OVERFLOW") == 0)
    return AtError::TEST_PARAM_OVERFLOW;
  if (strcmp(line, "AT_NO_NETWORK_JOINED") == 0)
    return AtError::NO_NET_JOINED;
  if (strcmp(line, "AT_RX_ERROR") == 0)
    return AtError::RX_ERROR;
  if (strcmp(line, "AT_NO_CLASS_B_ENABLED") == 0)
    return AtError::NO_CLASS_B_ENABLE;
  if (strcmp(line, "AT_DUTYCYCLE_RESTRICTED") == 0)
    return AtError::DUTY_CYCLE_RESTRICT;
  if (strcmp(line, "AT_CRYPTO_ERROR") == 0)
    return AtError::CRYPTO_ERROR;
  if (strcmp(line, "AT_LIB_ERROR") == 0)
    return AtError::LIBRARY_ERROR;
  if (strcmp(line, "AT_TX_TIMEOUT") == 0)
    return AtError::TX_TIMEOUT;
  if (strcmp(line, "AT_RX_TIMEOUT") == 0)
    return AtError::RX_TIMEOUT;
  if (strcmp(line, "AT_RECONF_ERROR") == 0)
    return AtError::RECONF_ERROR;
  if (strcmp(line, "BOOTALERT") == 0)
    return AtError::BOOT_ALERT;

  return AtError::UNKNOWN;
}

void LSM1x0A_AtParser::eatBuffer(const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    char c = (char)data[i];
    // Detección de fin de línea estándar \r\n
    if (c == '\n' || c == '\r') {
      if (_lineIdx > 0) {
        _lineBuffer[_lineIdx] = '\0'; // Null-terminate
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

// -----------------------------------------------------------
// PARSER PRINCIPAL DE EVENTOS
// -----------------------------------------------------------
void LSM1x0A_AtParser::processLine(char* line)
{
  // 1. LIMPIEZA
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n'))
    line[--len] = '\0';
  if (len == 0)
    return;

  // Verbose logging of all received lines
  if (_eventCallback)
    _eventCallback(LsmEvent::VERBOSE, line, _eventCtx);

  // 2. DETECCIÓN DE EVENTOS ASÍNCRONOS (+EVT)
  // Basado en las funciones: AT_event_join, AT_event_receive, AT_event_confirm
  char* evtPtr = strstr(line, "+EVT:");

  if (evtPtr != NULL) {
    char* payload = evtPtr + 5; // Saltamos el prefijo "+EVT:"
    if (_eventCallback) {
      // --- A. JOIN ---
      if (strncmp(payload, "JOINED", 6) == 0) {
        _eventCallback(LsmEvent::JOIN, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "JOIN FAILED", 11) == 0) {
        _eventCallback(LsmEvent::JOIN, "FAILED", _eventCtx);
      }

      // --- B. RECEPCIÓN DE DATOS (RECV) ---
      // Formato: RECV_CONFIRMED:PORT:SIZE:DATA...
      else if (strncmp(payload, "RECV_", 5) == 0) {
        // Buscamos el primer ':' para saltar CONFIRMED/UNCONFIRMED
        char* ptr = strchr(payload, ':');
        if (ptr) {
          // Pasamos "PORT:SIZE:DATA" limpio al usuario
          _eventCallback(LsmEvent::RX_DATA, ptr + 1, _eventCtx);
        }
      }

      // --- C. METADATOS DE RECEPCIÓN (RX_) ---
      // Formato: RX_1, PORT 2, DR 5, RSSI -90, SNR 10...
      // OJO: No confundir con RECV_. Comprobamos RX_
      else if (strncmp(payload, "RX_", 3) == 0) {
        // Pasamos todo el string de metadatos para que el helper lo parsee
        _eventCallback(LsmEvent::RX_META, payload, _eventCtx);
      }

      // --- D. CONFIRMACIÓN DE ENVÍO (TX) ---
      else if (strncmp(payload, "SEND_CONFIRMED", 14) == 0) {
        _eventCallback(LsmEvent::TX, "SUCCESS", _eventCtx);
      }
      else if (strncmp(payload, "SEND_FAILED", 11) == 0) {
        _eventCallback(LsmEvent::TX, "FAILED", _eventCtx);
      }

      // --- E. CAMBIO DE CLASE ---
      else if (strncmp(payload, "SWITCH_TO_CLASS_", 16) == 0) {
        // Payload será "SWITCH_TO_CLASS_C", pasamos "C"
        _eventCallback(LsmEvent::CLASS, payload + 16, _eventCtx);
      }

      // --- F. BEACONS ---
      else if (strncmp(payload, "BEACON_LOST", 11) == 0) {
        _eventCallback(LsmEvent::BEACON, "LOST", _eventCtx);
      }
      else if (strncmp(payload, "RX_BC", 5) == 0) {
        // Beacon recibido con info
        _eventCallback(LsmEvent::BEACON, payload, _eventCtx);
      }
    }
    return; // Evento procesado, no seguimos buscando respuestas de comandos
  }

  // 3. CASOS ESPECIALES SIN PREFIJO +EVT
  if (strncmp(line, "NVM DATA", 8) == 0) {
    if (_eventCallback)
      _eventCallback(LsmEvent::NVM, line, _eventCtx);
    return;
  }

  // 4. Ignorar líneas irrelevantes
  // Descartamos ecos asincronos del propio comando enviado (e.g. "AT+BAT=?")
  if (strncmp(line, "AT+", 3) == 0)
    return;

  // Si llegamos aquí, la línea NO tenía +EVT, así que si empieza por "confirmed flag",
  // es basura aislada y la descartamos.
  if (strncmp(line, "confirmed flag:", 15) == 0)
    return;
  if (strncmp(line, "---", 3) == 0)
    return; // Logs de boot
  if (strncmp(line, "+EVT:Prepare Frame", 18) == 0)
    return;

  // Detección del tipo de dispositivo
  if (strstr(line, "LSM100A") != nullptr)
    _deviceType = LsmModuleType::LSM100A;
  else if (strstr(line, "LSM110A") != nullptr)
    _deviceType = LsmModuleType::LSM110A;

  // Interceptar MAC rxTimeOut explícitamente sin fallar el pendingCommand
  if (strstr(line, "MAC rxTimeOut") != nullptr) {
    if (_eventCallback)
      _eventCallback(LsmEvent::RX_TIMEOUT, "TIMEOUT", _eventCtx);
  }

  if (_pendingCommand) {
    AtError err = parseErrorString(line);

    // Si la linea es un código de retorno final (OK, ERROR, etc... finalizamos)
    if (err != AtError::UNKNOWN) {
      _lastResultError = err;
      xSemaphoreGive(_syncSem);
      return;
    }

    // Captura de datos (Getters). Solo si la línea NO era un OK ni Error ni Eco
    if (_userOutBuffer != nullptr) {
      const char* dataStart = line;
      bool        match     = (_expectedTag == nullptr); // Si no hay tag, cogemos toda la línea

      if (_expectedTag && strstr(line, _expectedTag)) {
        dataStart = strstr(line, _expectedTag) + strlen(_expectedTag);
        match     = true;
      }

      if (match) {
        while (*dataStart == ' ' || *dataStart == ':')
          dataStart++; // Limpiamos prefijos
        strncpy(_userOutBuffer, dataStart, _userOutSize - 1);
        _userOutBuffer[_userOutSize - 1] = '\0';
      }
    }
  }
}

// -----------------------------------------------------------
// HELPER PARA PARSEAR METADATOS DE COBERTURA
// -----------------------------------------------------------
bool LSM1x0A_AtParser::parseRxMetadata(const char* payload, LsmRxMetadata* out)
{
  // Ejemplo: "RX_1, PORT 2, DR 5, RSSI -90, SNR 10, DMODM 10, GWN 2"
  if (!payload || !out)
    return false;

  // Inicializar defaults
  memset(out, 0, sizeof(LsmRxMetadata));
  out->hasLinkCheck = false;

  // 1. Extraer Slot (RX_1, RX_C, etc)
  // El payload empieza por "RX_"
  const char* pStart = payload + 3; // Saltar "RX_"
  const char* pComma = strchr(pStart, ',');
  if (pComma) {
    int len = pComma - pStart;
    if (len < sizeof(out->slot)) {
      strncpy(out->slot, pStart, len);
      out->slot[len] = '\0';
    }
  }
  else
    return false; // Formato inválido

  // 2. Extraer campos numéricos estándar
  // Buscamos substrings clave
  const char* pPort = strstr(payload, "PORT ");
  const char* pDr   = strstr(payload, "DR ");
  const char* pRssi = strstr(payload, "RSSI ");
  const char* pSnr  = strstr(payload, "SNR ");

  if (pPort)
    out->port = atoi(pPort + 5);
  if (pDr)
    out->dataRate = atoi(pDr + 3);
  if (pRssi)
    out->rssi = atoi(pRssi + 5);
  if (pSnr)
    out->snr = atoi(pSnr + 4);

  // 3. Extraer LinkCheck (Opcional)
  const char* pDmodm = strstr(payload, "DMODM ");
  const char* pGwn   = strstr(payload, "GWN ");

  if (pDmodm && pGwn) {
    out->hasLinkCheck = true;
    out->demodMargin  = atoi(pDmodm + 6);
    out->nbGateways   = atoi(pGwn + 4);
  }

  return true;
}

bool LSM1x0A_AtParser::wakeUp(uint8_t retries, uint32_t delayMs)
{
  _driver->flushRx();
  for (uint8_t i = 0; i < retries; i++) {
    // Enviar Ping silenciando los errores de payload/buffer para no pisar lecturas.
    AtError err = sendCommand("AT", delayMs);
    if (err == AtError::OK) {
      return true;
    }
  }
  return false;
}

AtError LSM1x0A_AtParser::sendCommand(const char* cmd, uint32_t timeoutMs)
{
  return sendCommandWithResponse(cmd, nullptr, nullptr, 0, timeoutMs);
}

AtError LSM1x0A_AtParser::waitForEvent(uint32_t timeoutMs)
{
  xSemaphoreTake(_syncSem, 0); // Limpiar

  if (_pendingCommand) {
    return AtError::BUSY;
  }

  _pendingCommand  = true;
  _lastResultError = AtError::TIMEOUT;

  if (xSemaphoreTake(_syncSem, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
    _pendingCommand = false;
    return AtError::TIMEOUT;
  }

  _pendingCommand = false;
  return _lastResultError;
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
  _driver->sendData("\r\n", 2);

  // 4. Bloquear esperando respuesta
  if (xSemaphoreTake(_syncSem, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
    _pendingCommand = false;
    return AtError::TIMEOUT;
  }

  _pendingCommand = false;
  _userOutBuffer  = nullptr; // Desvincular buffer por seguridad

  return _lastResultError;
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
  if (err == AtError::NO_CLASS_B_ENABLE)
    return "AT_NO_CLASS_B_ENABLED";
  if (err == AtError::DUTY_CYCLE_RESTRICT)
    return "AT_DUTYCYCLE_RESTRICTED";
  if (err == AtError::CRYPTO_ERROR)
    return "AT_CRYPTO_ERROR";
  if (err == AtError::LIBRARY_ERROR)
    return "AT_LIB_ERROR";
  if (err == AtError::TX_TIMEOUT)
    return "AT_TX_TIMEOUT";
  if (err == AtError::RX_TIMEOUT)
    return "AT_RX_TIMEOUT";
  if (err == AtError::RECONF_ERROR)
    return "AT_RECONF_ERROR";
  if (err == AtError::BOOT_ALERT)
    return "BOOTALERT";
  if (err == AtError::TIMEOUT)
    return "TIMEOUT";
  if (err == AtError::BUFFER_OVERFLOW)
    return "BUFFER_OVERFLOW";
  return "UNKNOWN";
}