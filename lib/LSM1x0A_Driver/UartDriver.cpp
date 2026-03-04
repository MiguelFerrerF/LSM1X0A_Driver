#include "UartDriver.h"

#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024
#define EVENT_QUEUE_SIZE 20

bool UartDriver::init(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, ReceiveCallback callback, void* context)
{
  if (callback == nullptr) {
    LSM_LOG_ERROR("UART", "UartDriver ERROR: Receive callback cannot be null.");
    return false;
  }

  _uart_port        = uart_num;
  _receive_callback = callback;
  _callback_context = context;

  uart_config_t uart_config = {
    .baud_rate  = baud_rate,
    .data_bits  = UART_DATA_8_BITS,
    .parity     = UART_PARITY_DISABLE,
    .stop_bits  = UART_STOP_BITS_1,
    .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
  };

  // Initialize the UART driver with the specified parameters and set up the RX task to handle incoming data asynchronously.
  if (uart_param_config(_uart_port, &uart_config) != ESP_OK) {
    LSM_LOG_ERROR("UART", "UartDriver ERROR: Could not configure port %d.", uart_num);
    return false;
  }

  if (uart_set_pin(_uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
    LSM_LOG_ERROR("UART", "UartDriver ERROR: Could not configure pins for port %d.", uart_num);
    return false;
  }

  if (uart_driver_install(_uart_port, RX_BUF_SIZE, TX_BUF_SIZE, EVENT_QUEUE_SIZE, &uart_event_queue, 0) != ESP_OK) {
    LSM_LOG_ERROR("UART", "UartDriver ERROR: Could not install driver for port %d.", uart_num);
    return false;
  }

  // Create the task with a slightly higher priority to avoid losing bytes in case the user task is busy
  xTaskCreate(rx_task_entry, "uart_rx_task", 4096, this, 12, &rx_task_handle);

  LSM_LOG_INFO("UART", "UartDriver: Initialized on port %d, Baudrate %d.", uart_num, baud_rate);
  return true;
}

int UartDriver::sendData(const char* data, size_t len)
{
  // --- ESP-IDF Sending Logic ---
  return uart_write_bytes(_uart_port, data, len);
}

bool UartDriver::deinit()
{
  if (rx_task_handle != nullptr) {
    vTaskDelete(rx_task_handle);
  }
  if (uart_driver_delete(_uart_port) != ESP_OK) {
    LSM_LOG_ERROR("UART", "UartDriver ERROR: Could not uninstall driver for port %d.", _uart_port);
    return false;
  }
  LSM_LOG_INFO("UART", "UartDriver: Port %d uninitialized.", _uart_port);
  return true;
}

void UartDriver::flushRx()
{
  // Clear the hardware RX buffer to discard any stale data that might have been received but not yet processed.
  uart_flush_input(_uart_port);

  // Clear the FreeRTOS event queue to ensure no old UART_DATA events are pending. This is important to avoid processing stale data after a flush.
  if (uart_event_queue != nullptr) {
    xQueueReset(uart_event_queue);
  }
}

// Entry point implementation for the FreeRTOS task
void UartDriver::rx_task_entry(void* pvParameters)
{
  // The task obtains a reference to the UartDriver object and executes its loop.
  static_cast<UartDriver*>(pvParameters)->rx_task_loop();
}

// Asynchronous data reception logic
void UartDriver::rx_task_loop()
{
  uart_event_t event;
  uint8_t      dtmp[RX_BUF_SIZE];

  while (true) {
    // --- ESP-IDF Reading Logic ---
    // Wait for an event from the UART queue
    if (xQueueReceive(uart_event_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
      // Clear temporary buffer
      memset(dtmp, 0, RX_BUF_SIZE);
      switch (event.type) {
        // --- NORMAL CASE: DATA RECEIVED ---
        case UART_DATA:
          // Read exactly the number of bytes indicated by the event
          uart_read_bytes(_uart_port, dtmp, event.size, portMAX_DELAY);

          // Invoke callback passing the context
          if (_receive_callback) {
            _receive_callback(_callback_context, dtmp, event.size);
          }
          break;

        // --- ERROR CASES (Robustness) ---
        case UART_FIFO_OVF:
          LSM_LOG_ERROR("UART", "Error: UART FIFO Overflow. Hardware saturated.");
          uart_flush_input(_uart_port); // Clear to recover state
          xQueueReset(uart_event_queue);
          break;

        case UART_BUFFER_FULL:
          LSM_LOG_ERROR("UART", "Error: UART Ring Buffer Full. Software too slow.");
          uart_flush_input(_uart_port);
          xQueueReset(uart_event_queue);
          break;

        case UART_BREAK:
          // Sometimes useful to detect disconnections or resets
          break;

        case UART_PARITY_ERR:
        case UART_FRAME_ERR:
          LSM_LOG_ERROR("UART", "Error: Line noise (Parity/Frame error).");
          break;

        default:
          break;
      }
    }
  }
  vTaskDelete(NULL);
}