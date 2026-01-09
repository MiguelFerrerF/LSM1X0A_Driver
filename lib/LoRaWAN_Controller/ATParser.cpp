#include "ATParser.h"
#include <cstring>
#include <iostream>

ATParser::ATParser() : _lineIdx(0), _eventCallback(nullptr) {
  _expectation.active = false;
  _expectation.syncSem = NULL;
  _expectation.dataBuffer = nullptr;
}

ATParser::~ATParser() {
  if (_expectation.syncSem != NULL) {
    vSemaphoreDelete(_expectation.syncSem);
  }
}

bool ATParser::init() {
  _expectation.syncSem = xSemaphoreCreateBinary();
  return (_expectation.syncSem != NULL);
}

void ATParser::registerEventCallback(ATEventCallback cb) {
  _eventCallback = cb;
}

SemaphoreHandle_t ATParser::getSemaphore() { return _expectation.syncSem; }

ATResult_t ATParser::getStatus() { return _expectation.status; }

void ATParser::setExpectation(ATCmdType_t type, char *buffer, uint16_t size) {
  xSemaphoreTake(_expectation.syncSem, 0);
  _expectation.type = type;
  _expectation.dataBuffer = buffer;
  _expectation.bufferSize = size;
  _expectation.status = AT_RES_PENDING;
  _expectation.active = true;
}

void ATParser::abort() {
  _expectation.active = false;
  _expectation.status = AT_RES_ABORTED;
}

void ATParser::_clearBuffer() {
  _lineIdx = 0;
  _lineBuffer[0] = '\0';
}

void ATParser::processByte(char c) {
  if (c == 0)
    return;

  std::cout << "Received char: " << c << std::endl;

  if (c == '\n' || c == '\r') {
    if (_lineIdx > 0) {
      _lineBuffer[_lineIdx] = '\0';
      _handleLine();
      _clearBuffer();
    }
    return;
  }

  if (_lineIdx < (AT_MAX_LINE_SIZE - 1)) {
    if (c >= 32 && c <= 126) {
      _lineBuffer[_lineIdx++] = c;
    }
  }
}

void ATParser::_handleLine() {

  // 1. Eventos Asíncronos (URC) - Prioridad Máxima
  if (strncmp(_lineBuffer, STR_AT_EVT_PREFIX, strlen(STR_AT_EVT_PREFIX)) == 0) {
    if (_eventCallback) {
      _eventCallback(_lineBuffer);
    }
    return;
  }

  // 2. Gestión de Expectativas
  if (_expectation.active) {

    // --- NUEVO: Manejo del Modo BOOT ---
    if (_expectation.type == AT_TYPE_BOOT) {
      // A. Buscar el tag de versión (ej: ">> Device: LSM100A")
      // Según Doc3/4 el formato incluye "Device" o "Device:"
      char *devPtr = strstr(_lineBuffer, STR_AT_DEVICE_TAG);
      if (devPtr != nullptr && _expectation.dataBuffer != nullptr) {
        // Avanzar puntero después de "Device:" y espacios
        devPtr += strlen(STR_AT_DEVICE_TAG);
        while (*devPtr == ' ' || *devPtr == '\t')
          devPtr++;

        // Copiar al buffer del usuario (versión detectada)
        strncpy(_expectation.dataBuffer, devPtr, _expectation.bufferSize - 1);
        _expectation.dataBuffer[_expectation.bufferSize - 1] = '\0';
      }

      // B. Esperar el terminador "BOOTALERT"
      if (strstr(_lineBuffer, STR_AT_BOOT_ALERT) != nullptr) {
        _expectation.status = AT_RES_OK;
        _expectation.active = false;
        xSemaphoreGive(_expectation.syncSem);
      }
      // En modo BOOT, ignoramos otras líneas sin error
      return;
    }

    // --- Manejo Estándar (EXEC / QUERY) ---

    // OK
    if (strcmp(_lineBuffer, STR_AT_OK) == 0) {
      _expectation.status = AT_RES_OK;
      _expectation.active = false;
      xSemaphoreGive(_expectation.syncSem);
      return;
    }

    // ERROR
    if (strstr(_lineBuffer, STR_AT_ERROR) != nullptr ||
        strstr(_lineBuffer, STR_AT_ERROR_ALT) != nullptr) {
      _expectation.status = AT_RES_ERROR;
      _expectation.active = false;
      xSemaphoreGive(_expectation.syncSem);
      return;
    }

    // DATA (Query)
    if (_expectation.type == AT_TYPE_QUERY &&
        _expectation.dataBuffer != nullptr) {
      strncpy(_expectation.dataBuffer, _lineBuffer,
              _expectation.bufferSize - 1);
      _expectation.dataBuffer[_expectation.bufferSize - 1] = '\0';
    }
  }
}