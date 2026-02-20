#include "UartDriver.h"
#include <cstdio>
#include <cstring>

#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024
#define EVENT_QUEUE_SIZE 20

bool UartDriver::init(uart_port_t uart_num, int baud_rate, int rx_pin, int tx_pin, ReceiveCallback callback, void* context)
{
  if (callback == nullptr) {
    printf("UartDriver ERROR: Callback de recepción no puede ser nulo.\n");
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

  // --- Lógica de Inicialización de ESP-IDF ---
  if (uart_param_config(_uart_port, &uart_config) != ESP_OK) {
    printf("UartDriver ERROR: No se pudo configurar el puerto %d.\n", uart_num);
    return false;
  }

  if (uart_set_pin(_uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
    printf("UartDriver ERROR: No se pudieron configurar los pines del puerto "
           "%d.\n",
           uart_num);
    return false;
  }

  if (uart_driver_install(_uart_port, RX_BUF_SIZE, TX_BUF_SIZE, EVENT_QUEUE_SIZE, &uart_event_queue, 0) != ESP_OK) {
    printf("UartDriver ERROR: No se pudo instalar el driver del puerto %d.\n", uart_num);
    return false;
  }

  // Creamos la tarea con prioridad un poco más alta para no perder bytes
  xTaskCreate(rx_task_entry, "uart_rx_task", 4096, this, 12, &rx_task_handle);
  // -----------------------------------------------------------------

  printf("UartDriver: Inicializado en puerto %d, Baudrate %d.\n", uart_num, baud_rate);
  return true;
}

int UartDriver::sendData(const char* data, size_t len)
{
  // --- Lógica de Envío de ESP-IDF ---
  return uart_write_bytes(_uart_port, data, len);
}

bool UartDriver::deinit()
{
  if (rx_task_handle != nullptr) {
    vTaskDelete(rx_task_handle);
  }
  if (uart_driver_delete(_uart_port) != ESP_OK) {
    printf("UartDriver ERROR: No se pudo desinstalar el driver del puerto %d.\n", _uart_port);
    return false;
  }
  printf("UartDriver: Desinstalado del puerto %d.\n", _uart_port);
  return true;
}

void UartDriver::flushRx()
{
  // 1. Limpiamos las colas y buffers internos de hardware de ESP-IDF
  uart_flush_input(_uart_port);
  
  // 2. Vaciamos la cola de eventos de FreeRTOS asegurándonos de que 
  // no queden pendientes eventos UART_DATA antiguos.
  if (uart_event_queue != nullptr) {
      xQueueReset(uart_event_queue);
  }
}

// Implementación del punto de entrada para la tarea de FreeRTOS
void UartDriver::rx_task_entry(void* pvParameters)
{
  // La tarea obtiene una referencia al objeto UartDriver y ejecuta su bucle.
  static_cast<UartDriver*>(pvParameters)->rx_task_loop();
}

// Lógica de recepción asíncrona de datos
void UartDriver::rx_task_loop()
{
  uart_event_t event;
  uint8_t      dtmp[RX_BUF_SIZE];

  while (true) {
    // --- Lógica de Lectura de ESP-IDF ---
    // Esperamos por un evento de la cola de la UART
    if (xQueueReceive(uart_event_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
      // Limpiamos buffer temporal
      memset(dtmp, 0, RX_BUF_SIZE);
      switch (event.type) {
        // --- CASO NORMAL: LLEGARON DATOS ---
        case UART_DATA:
          // Leemos exactamente la cantidad de datos que el evento dice que hay
          uart_read_bytes(_uart_port, dtmp, event.size, portMAX_DELAY);

          // Invocamos callback pasando el contexto
          if (_receive_callback) {
            _receive_callback(_callback_context, dtmp, event.size);
          }
          break;

        // --- CASOS DE ERROR (Robustez) ---
        case UART_FIFO_OVF:
          printf("Error: UART FIFO Overflow. Hardware saturado.\n");
          uart_flush_input(_uart_port); // Limpiar para recuperar estado
          xQueueReset(uart_event_queue);
          break;

        case UART_BUFFER_FULL:
          printf("Error: UART Ring Buffer Full. Software lento.\n");
          uart_flush_input(_uart_port);
          xQueueReset(uart_event_queue);
          break;

        case UART_BREAK:
          // A veces útil para detectar desconexiones o resets
          break;

        case UART_PARITY_ERR:
        case UART_FRAME_ERR:
          printf("Error: Ruido en linea (Parity/Frame error).\n");
          break;

        default:
          break;
      }
    }
  }
  vTaskDelete(NULL);
}