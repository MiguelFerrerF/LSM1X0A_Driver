#include "UartDriver.h"
#include "freertos/semphr.h"
#include "unity.h"
#include <Arduino.h>
#include <cstring>

#define TEST_UART_PORT UART_NUM_2
#define TEST_TX_PIN 33
#define TEST_RX_PIN 14
#define TEST_BAUD 9600

// ==========================================
// HELPER: Estructura de Contexto para el Test
// ==========================================
// Esta estructura nos permite verificar que el puntero void* funciona
// y nos sirve para sincronizar el test con la interrupción.
struct TestContext
{
  uint8_t           buffer[1024];
  size_t            received_len;
  SemaphoreHandle_t sync_sem; // Semáforo para despertar al Test
  int               callback_count;

  TestContext()
  {
    received_len   = 0;
    callback_count = 0;
    memset(buffer, 0, 1024);
    sync_sem = xSemaphoreCreateBinary();
  }

  ~TestContext()
  {
    vSemaphoreDelete(sync_sem);
  }

  void reset()
  {
    received_len   = 0;
    callback_count = 0;
    memset(buffer, 0, 1024);
    xSemaphoreTake(sync_sem, 0); // Limpiar semáforo
  }
};

// ==========================================
// CALLBACK ESTÁTICO (El que llama el Driver)
// ==========================================
static void test_rx_callback(void* ctx_void, uint8_t* data, size_t len)
{
  TestContext* ctx = (TestContext*)ctx_void;

  // Verificación defensiva
  if (ctx == nullptr || len > 1024)
    return;

  // Copiar datos al buffer del contexto
  memcpy(ctx->buffer, data, len);
  ctx->received_len = len;
  ctx->callback_count++;

  // Imprimir datos recibidos (opcional, para debug)
  Serial.print("Callback recibido: ");
  for (size_t i = 0; i < len; i++) {
    Serial.print((char)data[i]);
  }
  // ¡Despierta al Test Runner!
  xSemaphoreGive(ctx->sync_sem);
}

// ==========================================
// CASO DE PRUEBA 1: Comunicación Básica (Loopback)
// ==========================================
void test_uart_loopback_basic()
{
  UartDriver  driver;
  TestContext ctx;

  // 1. Inicializar
  bool init_ok = driver.init(TEST_UART_PORT, TEST_BAUD, TEST_RX_PIN, TEST_TX_PIN, test_rx_callback, &ctx);
  TEST_ASSERT_TRUE_MESSAGE(init_ok, "Fallo al inicializar UART Driver");

  // 2. Preparar datos
  const char* rx_msg  = "\r\nAT_ERROR\r\n";
  const char* msg     = "AT\r\n";
  size_t      msg_len = strlen(rx_msg);

  // 3. Enviar datos (físicamente salen por TX y entran por RX)
  int sent_bytes = driver.sendData((uint8_t*)msg, strlen(msg));

  // 4. Volver a enviar los datos para asegurar que el buffer no se pierde
  // en caso de que el primer envío falle.
  sent_bytes = driver.sendData((uint8_t*)msg, strlen(msg));

  TEST_ASSERT_EQUAL(strlen(msg), sent_bytes);

  // 4. Esperar a que la tarea del driver procese la recepción
  // Damos un timeout de 200ms (suficiente para 9600 baudios)
  BaseType_t res = xSemaphoreTake(ctx.sync_sem, pdMS_TO_TICKS(500));

  TEST_ASSERT_TRUE_MESSAGE(res == pdTRUE, "Timeout: No se recibio respuesta (¿Esta el cable Loopback conectado?)");

  // 5. Verificar integridad
  TEST_ASSERT_EQUAL_INT(msg_len, ctx.received_len);
  TEST_ASSERT_EQUAL_MEMORY(rx_msg, ctx.buffer, msg_len);
  TEST_ASSERT_EQUAL_INT(1, ctx.callback_count);

  driver.deinit();
}

// ==========================================
// CASO DE PRUEBA 2: Robustez (Init/Deinit Cíclico)
// ==========================================
void test_uart_lifecycle()
{
  UartDriver  driver;
  TestContext ctx;

  // Intentamos iniciar y detener varias veces para detectar fugas de memoria o crashes
  for (int i = 0; i < 3; i++) {
    bool ok = driver.init(TEST_UART_PORT, TEST_BAUD, TEST_RX_PIN, TEST_TX_PIN, test_rx_callback, &ctx);
    TEST_ASSERT_TRUE(ok);

    // Pequeño envío para asegurar que la tarea arrancó
    driver.sendData((uint8_t*)"A", 1);
    xSemaphoreTake(ctx.sync_sem, pdMS_TO_TICKS(100));

    driver.deinit();

    // Pequeña pausa para que FreeRTOS limpie recursos
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// ==========================================
// CASO DE PRUEBA 3: Manejo de Errores (Configuración Inválida)
// ==========================================
void test_uart_invalid_config()
{
  UartDriver driver;

  // 1. Probar callback nulo
  bool res = driver.init(TEST_UART_PORT, 115200, TEST_RX_PIN, TEST_TX_PIN, nullptr, nullptr);
  TEST_ASSERT_FALSE_MESSAGE(res, "Debería fallar si el callback es NULL");

  // 2. Probar pines inválidos (ej. pin -1 o pines reservados, aunque ESP-IDF a veces lo permite,
  // probamos un puerto UART que no existe si es posible o configuración absurda)
  // Nota: ESP32 tiene UART 0, 1, 2. UART_NUM_MAX suele fallar.
  res = driver.init(UART_NUM_MAX, 9600, TEST_RX_PIN, TEST_TX_PIN, test_rx_callback, nullptr);
  TEST_ASSERT_FALSE_MESSAGE(res, "Debería fallar con puerto UART inválido");
}

// ==========================================
// RUNNER
// ==========================================

int custom_main()
{
  // Esperar un poco al arranque para que el monitor serie se conecte
  vTaskDelay(pdMS_TO_TICKS(2000));

  UNITY_BEGIN();

  RUN_TEST(test_uart_loopback_basic);
  RUN_TEST(test_uart_lifecycle);
  RUN_TEST(test_uart_invalid_config);

  return UNITY_END();
}

void setup()
{
  // Arduino Setup
  Serial.begin(115200);
  custom_main();
}

void loop()
{
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
void setUp()
{
}

void tearDown()
{
}
