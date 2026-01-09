#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define READ_BUFFER_SIZE 256
#define INTERNAL_BUFFER_SIZE 256

// Pointer to a function that handles received data
using OnReceiveCallback = void (*)(const uint8_t *data, size_t len);

class SerialDriver {
public:
  /**
   * @brief Construct a new Serial Driver object
   *
   * @param serialPort  Reference to the HardwareSerial port to use
   */
  SerialDriver(HardwareSerial &serialPort);

  /**
   * @brief Destroy the Serial Driver object
   *
   * This destructor stops the serial reading task if it is running.
   */
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
   * @param callback The function to call when data is received.
   * @return true if the task was successfully created and started.
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

  /**
   * @brief Writes data to the serial port.
   *
   * Sends the specified buffer of data to the serial port.
   *
   * @param buffer Pointer to the data buffer to send.
   * @param size Size of the data buffer in bytes.
   * @return Number of bytes successfully written.
   */
  size_t write(const uint8_t *buffer, size_t size);

  /**
   * @brief Writes data to the serial port.
   *
   * Overloaded function to send a buffer of data (as a character array) to the
   * serial port.
   *
   * @param buffer Pointer to the character array to send.
   * @param size Size of the character array in bytes.
   * @return Number of bytes successfully written.
   */
  size_t write(const char *buffer, size_t size) {
    return write((const uint8_t *)buffer, size);
  }

  /**
   * @brief Get the Task Handle object
   *
   * @return true
   * @return false
   */
  bool getTaskHandle() const { return _taskHandle == NULL ? false : true; }

private:
  HardwareSerial &_port;
  TaskHandle_t _taskHandle = NULL;
  OnReceiveCallback _onReceiveCallback = nullptr;
  bool _running = false;

  uint8_t _msgBuffer[INTERNAL_BUFFER_SIZE] = {0};
  size_t _msgIndex = 0;

  static void readTask(void *ctx);
  void readLoop();
};

#endif // SERIAL_DRIVER_H