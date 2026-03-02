#ifndef UARTDRIVER_H
#define UARTDRIVER_H

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <cstdint>
#include <cstdlib>

// Definimos el callback para incluir un contexto (void*).
// Esto permite llamar métodos de clases C++ desde este driver C-style.
using ReceiveCallback = void (*)(void* ctx, uint8_t* data, size_t len);

/**
 * @brief Simple UART hardware driver for Espressif chips using ESP-IDF.
 * It provides asynchronous receive capabilities using FreeRTOS tasks to feed data callbacks.
 */
class UartDriver
{
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
   * @param context Contexto a pasar al callback.
   * @return true si la inicialización fue exitosa.
   */
  bool init(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, ReceiveCallback callback, void* context = nullptr);

  /**
   * @brief Envía datos a través de la UART.
   * @param data Puntero a los datos a enviar.
   * @param len Longitud de los datos.
   * @return Número de bytes escritos, o -1 en caso de error.
   */
  int sendData(const char* data, size_t len);

  /**
   * @brief Envía datos a través de la UART.
   * @param data Puntero a los datos a enviar.
   * @param len Longitud de los datos.
   * @return Número de bytes escritos, o -1 en caso de error.
   */
  int sendData(const uint8_t* data, size_t len)
  {
    return sendData((const char*)data, len);
  }

  /**
   * @brief Desinstala el driver de UART.
   * @return true si la desinstalación fue exitosa.
   */
  bool deinit();

  /**
   * @brief Vacia el buffer de RX de hardware y cualquier evento encolado,
   * descartando todos los datos obsoletos recibidos.
   */
  void flushRx();

private:
  uart_port_t     _uart_port        = UART_NUM_MAX;
  ReceiveCallback _receive_callback = nullptr;
  void*           _callback_context = nullptr; // Guardamos quién nos llamó

  TaskHandle_t  rx_task_handle   = nullptr;
  QueueHandle_t uart_event_queue = nullptr;

  // Función estática que actúa como punto de entrada de la tarea de recepción
  // de ESP-IDF
  static void rx_task_entry(void* pvParameters);

  // Lógica principal de la tarea de recepción
  void rx_task_loop();
};

#endif // UARTDRIVER_H