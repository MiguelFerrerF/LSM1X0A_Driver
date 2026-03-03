#ifndef UARTDRIVER_H
#define UARTDRIVER_H

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <cstdint>
#include <cstdlib>

// We define the callback to include a context (void*).
// This allows calling C++ class methods from this C-style driver.
using ReceiveCallback = void (*)(void* ctx, uint8_t* data, size_t len);

/**
 * @brief Simple UART hardware driver for Espressif chips using ESP-IDF.
 * It provides asynchronous receive capabilities using FreeRTOS tasks to feed data callbacks.
 */
class UartDriver
{
public:
  /**
   * @brief UartDriver class constructor.
   */
  UartDriver() = default;

  /**
   * @brief Initializes the UART driver.
   * @param uart_num UART port number (e.g. UART_NUM_2).
   * @param baud_rate Transmission speed (e.g. 9600).
   * @param rx_pin GPIO pin for reception.
   * @param tx_pin GPIO pin for transmission.
   * @param callback Function to call when data is received.
   * @param context Context to pass to the callback.
   * @return true if initialization was successful.
   */
  bool init(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, ReceiveCallback callback, void* context = nullptr);

  /**
   * @brief Sends data through UART.
   * @param data Pointer to the data to send.
   * @param len Length of the data.
   * @return Number of bytes written, or -1 in case of error.
   */
  int sendData(const char* data, size_t len);

  /**
   * @brief Sends data through UART.
   * @param data Pointer to the data to send.
   * @param len Length of the data.
   * @return Number of bytes written, or -1 in case of error.
   */
  int sendData(const uint8_t* data, size_t len)
  {
    return sendData((const char*)data, len);
  }

  /**
   * @brief Uninstalls the UART driver.
   * @return true if uninstallation was successful.
   */
  bool deinit();

  /**
   * @brief Empties the hardware RX buffer and any queued events,
   * discarding all obsolete received data.
   */
  void flushRx();

private:
  uart_port_t     _uart_port        = UART_NUM_MAX;
  ReceiveCallback _receive_callback = nullptr;
  void*           _callback_context = nullptr; // Guardamos quién nos llamó

  TaskHandle_t  rx_task_handle   = nullptr;
  QueueHandle_t uart_event_queue = nullptr;

  // Static function that acts as the entry point for the ESP-IDF receive task
  static void rx_task_entry(void* pvParameters);

  // Main logic of the receive task
  void rx_task_loop();
};

#endif // UARTDRIVER_H