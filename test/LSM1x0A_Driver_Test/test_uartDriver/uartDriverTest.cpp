#include "UartDriver.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
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

  // Copiar datos al buffer del contexto AFILANDO (Append)
  if (ctx->received_len + len < 1024) {
    memcpy(&ctx->buffer[ctx->received_len], data, len);
    ctx->received_len += len;
    // Nos aseguramos de que termine en null para poder usar strstr luego tranquilamente
    ctx->buffer[ctx->received_len] = '\0';
  }
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
// HELPER: Despertar al módulo
// ==========================================
void wake_up_module(UartDriver& driver, TestContext& ctx) {
    const char* atCommand = "AT\r\n";
    for(int i = 0; i < 3; i++) {
        driver.sendData((const uint8_t*)atCommand, strlen(atCommand));
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    driver.flushRx();
    ctx.reset();
}

// ==========================================
// CASO DE PRUEBA 1: Comunicación Básica (Ping al Módulo)
// ==========================================
void test_uart_ping_module()
{
  UartDriver  driver;
  TestContext ctx;

  // 1. Inicializar
  bool init_ok = driver.init(TEST_UART_PORT, TEST_BAUD, TEST_RX_PIN, TEST_TX_PIN, test_rx_callback, &ctx);
  TEST_ASSERT_TRUE_MESSAGE(init_ok, "Fallo al inicializar UART Driver");

  // 2. Preparar datos
  const char* msg     = "AT\r\n";
  
  // El módulo normalmente hace echo del comando y luego responde,
  // O bien (si el echo está apagado), solo responde `\r\nOK\r\n`.
  // Asumimos que responde OK (veremos qué longitud llega).
  const char* expected_resp = "\r\nOK\r\n"; 

  // 3. Despertar al módulo de forma segura
  wake_up_module(driver, ctx);

  // 4. Enviar datos reales
  int sent_bytes = driver.sendData((const uint8_t*)msg, strlen(msg));
  TEST_ASSERT_EQUAL_INT(strlen(msg), sent_bytes);

  // 4. Esperar respuesta 
  BaseType_t res = xSemaphoreTake(ctx.sync_sem, pdMS_TO_TICKS(1000));
  TEST_ASSERT_TRUE_MESSAGE(res == pdTRUE, "Timeout: No se recibio respuesta del modulo");

  // 5. Verificar integridad. Buscamos que contenga OK.
  // Como la lectura puede fragmentarse, esperamos un rato más si es corta
  vTaskDelay(pdMS_TO_TICKS(100)); 
  
  // Imprimo lo recibido para debugging si falla
  char dbg_buf[128];
  snprintf(dbg_buf, sizeof(dbg_buf), "Received len=%d: '%s'", ctx.received_len, ctx.buffer);
  TEST_MESSAGE(dbg_buf);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(strstr((char*)ctx.buffer, "OK"), "No se encontro OK en la respuesta");

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
// CASO DE PRUEBA 4: Prueba de Flush RX
// ==========================================
void test_uart_flush_rx()
{
  UartDriver  driver;
  TestContext ctx;

  bool init_ok = driver.init(TEST_UART_PORT, TEST_BAUD, TEST_RX_PIN, TEST_TX_PIN, test_rx_callback, &ctx);
  TEST_ASSERT_TRUE(init_ok);

  // Despertar al módulo primero para asegurarnos de que procesará la basura
  wake_up_module(driver, ctx);

  const char* msg = "BASURA\r\n";
  
  // Enviamos basura al modulo para que responda un error.
  driver.sendData((uint8_t*)msg, strlen(msg));
  
  // Esperamos suficiente tiempo para que el hardware UART reciba la respuesta de error 
  // del modulo, pero sin que nuestra capa de aplicacion la consuma
  vTaskDelay(pdMS_TO_TICKS(500));
  
  // Limpiamos todo (descartamos el AT_ERROR que acaba de llegar)
  driver.flushRx();
  ctx.reset();

  // Ahora enviamos el comando bueno
  const char* good_msg = "AT\r\n";
  driver.sendData((uint8_t*)good_msg, strlen(good_msg));

  // Esperamos la recepción
  BaseType_t res = xSemaphoreTake(ctx.sync_sem, pdMS_TO_TICKS(1000));
  TEST_ASSERT_TRUE(res == pdTRUE);

  vTaskDelay(pdMS_TO_TICKS(100)); // Dar tiempo a copiar toda la cadena "OK"
  
  char dbg_buf2[128];
  snprintf(dbg_buf2, sizeof(dbg_buf2), "Test 4 Received len=%d: '%s'", ctx.received_len, ctx.buffer);
  TEST_MESSAGE(dbg_buf2);

  // Deberíamos ver el \r\nOK\r\n o el eco sin ningún AT_ERROR antes
  TEST_ASSERT_NOT_NULL(strstr((char*)ctx.buffer, "OK"));
  TEST_ASSERT_NULL(strstr((char*)ctx.buffer, "ERROR"));

  driver.deinit();
}


// ==========================================
// RUNNER
// ==========================================

int custom_main()
{
  // Esperar un poco al arranque para que el monitor serie se conecte
  vTaskDelay(pdMS_TO_TICKS(2000));

  UNITY_BEGIN();

  RUN_TEST(test_uart_ping_module);
  RUN_TEST(test_uart_lifecycle);
  RUN_TEST(test_uart_invalid_config);
  RUN_TEST(test_uart_flush_rx);

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
