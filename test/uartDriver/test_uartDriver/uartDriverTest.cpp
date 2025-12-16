#include "UartDriver.h"
#include "unity.h"
#include <Arduino.h>
#include <cstring>


void test_uart_driver_send_receive() {
  UartDriver uart_driver;

  // Variable para almacenar datos recibidos en el callback
  const uint8_t *received_data = nullptr;
  size_t received_len = 0;

  // Callback simulado para capturar datos recibidos
  auto receive_callback = [](const uint8_t *data, size_t len) {
    // Use static variables to store received data
    static const uint8_t *static_received_data = nullptr;
    static size_t static_received_len = 0;
    static_received_data = data;
    static_received_len = len;
  };

  // Inicializar el driver UART
  bool init_result = uart_driver.init(2, 9600, 14, 33, receive_callback);
  TEST_ASSERT_TRUE(init_result);

  // Datos de prueba para enviar
  const char *test_cmd = "AT+TEST\r\n";
  size_t test_cmd_len = strlen(test_cmd);

  // Enviar datos
  int bytes_sent = uart_driver.sendData(
      reinterpret_cast<const uint8_t *>(test_cmd), test_cmd_len);
  TEST_ASSERT_EQUAL(test_cmd_len, bytes_sent);

  // Simular recepción de datos (normalmente esto lo haría el hardware)
  // Aquí simplemente llamamos al callback directamente para la prueba
  receive_callback(reinterpret_cast<const uint8_t *>(test_cmd), test_cmd_len);

  // Verificar que los datos recibidos coinciden con los enviados
  TEST_ASSERT_NOT_NULL(received_data);
  TEST_ASSERT_EQUAL(test_cmd_len, received_len);
  TEST_ASSERT_EQUAL_MEMORY(test_cmd, received_data, test_cmd_len);

  // Desinicializar el driver UART
  uart_driver.deinit();
}

int custom_main() {
  Serial.begin(115200);
  UNITY_BEGIN();
  RUN_TEST(test_uart_driver_send_receive);
  return UNITY_END();
}

void setup() { custom_main(); }

void loop() { vTaskDelay(1000 / portTICK_PERIOD_MS); }

void setUp() {}

void tearDown() {}
