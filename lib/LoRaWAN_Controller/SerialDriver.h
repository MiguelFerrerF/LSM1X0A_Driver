#ifndef LORAWAN_CONTROLLER_SERIAL_DRIVER_H
#define LORAWAN_CONTROLLER_SERIAL_DRIVER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Pointer to a function that handles received data
using OnReceiveCallback = void (*)(const uint8_t *data, size_t len);

class SerialDriver {
public:
  SerialDriver(HardwareSerial &serialPort);
  ~SerialDriver();

  /**
   * @brief Starts the serial reading task.
   *
   * This function initializes the FreeRTOS task that continuously reads data
   * from the specified serial port at the given baud rate and pin
   * configuration.
   *
   * @param baudRate The baud rate for serial communication.
   * @param rxPin The GPIO pin number for RX.
   * @param txPin The GPIO pin number for TX.
   */
  bool init(unsigned long baudRate, int rxPin, int txPin,
            OnReceiveCallback callback);

  /**
   * @brief Stops the serial reading task.
   *
   * This function sets the running flag to false and deletes the FreeRTOS task
   * responsible for reading from the serial port.
   *
   */
  void stop();

  size_t write(const uint8_t *buffer, size_t size);

  // Get the task handle (for testing purposes)
  bool getTaskHandle() const { return _taskHandle == NULL ? false : true; }

private:
  HardwareSerial &_port;
  TaskHandle_t _taskHandle = NULL;
  OnReceiveCallback _onReceiveCallback = nullptr;
  bool _running = false;

  static const size_t INTERNAL_BUFFER_SIZE = 0xFF;
  uint8_t _msgBuffer[INTERNAL_BUFFER_SIZE];
  size_t _msgIndex = 0;

  const uint8_t READ_BUFFER_SIZE = 0xFF;

  static void readTask(void *ctx);
  void readLoop();
};

#endif // LORAWAN_CONTROLLER_SERIAL_DRIVER_H