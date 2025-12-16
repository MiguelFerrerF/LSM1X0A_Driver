#ifndef UARTDRIVER_H
#define UARTDRIVER_H

#include <cstdint>
#include <cstdlib>
#include <driver/uart.h>

#define BUF_SIZE 1024

// Definición de tipos de datos para la configuración de la UART de ESP-IDF
// (simulados aquí)
using uart_port_t = int;

// Alias para el tipo de función de callback: notifica al parser cuando se
// recibe data
using ReceiveCallback = void (*)(const uint8_t *data, size_t len);

class UartDriver {
public:
  /**
   * @brief Constructor de la clase UartDriver.
   */
  UartDriver() = default;

  /**
   * @brief Inicializa el driver de UART.
   * @param uart_num Número del puerto UART (ej. UART_NUM_2).
   * @param baud_rate Velocidad de transmisión (ej. 9600).
   * @param rx_pin Pin GPIO para recepción.
   * @param tx_pin Pin GPIO para transmisión.
   * @param callback Función a llamar al recibir datos.
   * @return true si la inicialización fue exitosa.
   */
  bool init(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin,
            ReceiveCallback callback);

  /**
   * @brief Envía datos a través de la UART.
   * @param data Puntero a los datos a enviar.
   * @param len Longitud de los datos.
   * @return Número de bytes escritos, o -1 en caso de error.
   */
  int sendData(const uint8_t *data, size_t len);

  /**
   * @brief Desinstala el driver de UART.
   */
  void deinit();

private:
  ReceiveCallback _receive_callback = nullptr;
  uart_port_t _uart_port = -1;

  TaskHandle_t rx_task_handle = nullptr;
  QueueHandle_t uart_queue = nullptr;

  // Función estática que actúa como punto de entrada de la tarea de recepción
  // de ESP-IDF
  static void rx_task_entry(void *pvParameters);

  // Lógica principal de la tarea de recepción
  void rx_task_loop();
};

#endif // UARTDRIVER_H