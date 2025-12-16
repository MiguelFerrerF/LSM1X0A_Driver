#include "SerialDriver.h"

SerialDriver::SerialDriver(HardwareSerial &port) : _port(port) {
  _taskHandle = NULL;
  _onReceiveCallback = nullptr;
  _running = false;
  _msgBuffer[0] = '\0';
  _msgIndex = 0;
}

SerialDriver::~SerialDriver() { stop(); }

bool SerialDriver::init(unsigned long baudRate, int rxPin, int txPin,
                        OnReceiveCallback callback) {
  _msgIndex = 0;
  if (_running)
    return true;

  if (callback == nullptr)
    return false;
  _onReceiveCallback = callback;

  // HardwareSerial configuration
  _port.setRxBufferSize(256);
  _port.begin(baudRate, SERIAL_8N1, rxPin, txPin);

  _running = true;
  xTaskCreatePinnedToCore(readTask,     // Task function
                          "LSM_Driver", // Task name
                          3072,         // Stack size
                          this,         // Pass 'this' as context
                          5, // Priority (medium-high to avoid data loss)
                          &_taskHandle,
                          1 // Core 1 (App Core)
  );
  if (_taskHandle == NULL) {
    _running = false;
    return false;
  }
  return true;
}

void SerialDriver::stop() {
  _running = false;
  vTaskDelay(pdMS_TO_TICKS(50)); // Wait for the task to finish
  if (_taskHandle != NULL)
    _taskHandle = NULL;
}

size_t SerialDriver::write(const uint8_t *buffer, size_t size) {
  return _port.write(buffer, size);
}

void SerialDriver::readTask(void *ctx) {
  static_cast<SerialDriver *>(ctx)->readLoop();
  vTaskDelete(NULL);
}

void SerialDriver::readLoop() {
  uint8_t rawBuffer[READ_BUFFER_SIZE]; // Temporary buffer for reading

  while (_running) {
    int availableBytes = _port.available();

    if (availableBytes > 0) {
      size_t bytesToRead = (availableBytes > READ_BUFFER_SIZE)
                               ? READ_BUFFER_SIZE
                               : availableBytes;
      size_t bytesRead = _port.readBytes(rawBuffer, bytesToRead);

      for (size_t i = 0; i < bytesRead; i++) {
        uint8_t c = rawBuffer[i];

        // Store in internal buffer
        if (_msgIndex < INTERNAL_BUFFER_SIZE - 1) {
          _msgBuffer[_msgIndex++] = c;
        } else {
          // Buffer overflow, reset
          _msgIndex = 0;
          continue;
        }

        // Check for end of message (newline)
        if (c == '\n') {
          if (_msgIndex > 0) {
            _msgBuffer[_msgIndex] = 0; // Null-terminate for safety
            if (_onReceiveCallback != nullptr)
              _onReceiveCallback(_msgBuffer, _msgIndex);

            _msgIndex = 0;
          }
        }
        continue;
      }
    } else
      vTaskDelay(pdMS_TO_TICKS(10)); // Evitar busy-waiting
  }
}