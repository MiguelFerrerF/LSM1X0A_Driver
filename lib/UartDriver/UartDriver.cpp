#include "UartDriver.h"
#include <cstdio>

bool UartDriver::init(uart_port_t uart_num, int baud_rate, int rx_pin,
                      int tx_pin, ReceiveCallback callback) {
  if (callback == nullptr) {
    printf("UartDriver ERROR: Callback de recepción no puede ser nulo.\n");
    return false;
  }

  _receive_callback = callback;
  _uart_port = uart_num;

  // --- Lógica de Inicialización de ESP-IDF ---
  // 1. Configurar los parámetros de la UART
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  uart_param_config(_uart_port, &uart_config);

  // 2. Instalar el driver de UART (con buffers RX/TX) y crear la cola de
  // eventos
  uart_driver_install(_uart_port, BUF_SIZE, BUF_SIZE, 10, &uart_queue, 0);

  // 3. Configurar los pines GPIO
  uart_set_pin(_uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);

  // 4. Crear la tarea de recepción asíncrona
  xTaskCreate(rx_task_entry, "uart_rx_task", 2048, this, 10, &rx_task_handle);
  // -----------------------------------------------------------------

  printf("UartDriver: Inicializado en puerto %d, Baudrate %d.\n", uart_num,
         baud_rate);
  return true;
}

int UartDriver::sendData(const uint8_t *data, size_t len) {
  // --- Lógica de Envío de ESP-IDF ---
  return uart_write_bytes(_uart_port, (const char *)data, len);
}

void UartDriver::deinit() {
  // --- Lógica de Desinstalación de ESP-IDF ---
  vTaskDelete(rx_task_handle);
  uart_driver_delete(_uart_port);
  // -----------------------------------------------------

  printf("UartDriver: Desinstalado.\n");
  _receive_callback = nullptr;
}

// Implementación del punto de entrada para la tarea de FreeRTOS
void UartDriver::rx_task_entry(void *pvParameters) {
  // La tarea obtiene una referencia al objeto UartDriver y ejecuta su bucle.
  static_cast<UartDriver *>(pvParameters)->rx_task_loop();
}

// Lógica de recepción asíncrona de datos
void UartDriver::rx_task_loop() {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  while (true) {
    // --- Lógica de Lectura de ESP-IDF ---
    int len = uart_read_bytes(_uart_port, data, (BUF_SIZE - 1),
                              20 / portTICK_PERIOD_MS);

    // Simulación: Esperar datos y si llegan, llamar al callback
    if (len > 0 && _receive_callback != nullptr) {
      _receive_callback(data, len);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}