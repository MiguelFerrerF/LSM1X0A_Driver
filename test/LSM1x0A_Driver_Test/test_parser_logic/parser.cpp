#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include <Arduino.h>
#include <unity.h>

// =========================================================
// MOCKS Y AYUDAS
// =========================================================

UartDriver       driver;
LSM1x0A_AtParser parser;

// Estructura para capturar qué eventos ha disparado el parser
struct TestSpy
{
  String lastType;
  String lastPayload;
  int    eventCount;

  void reset()
  {
    lastType    = "";
    lastPayload = "";
    eventCount  = 0;
  }
};

TestSpy spy;

// Callback que inyectaremos al parser
void spyCallback(const char* type, const char* payload, void* ctx)
{
  TestSpy* s     = (TestSpy*)ctx;
  s->lastType    = String(type);
  s->lastPayload = String(payload);
  s->eventCount++;
  // Serial.printf("[SPY] Event: %s Data: %s\n", type, payload);
}

// Tarea simuladora de Módem para responder a comandos síncronos
void modemSimulatorTask(void* param)
{
  // Esperamos un poco para simular tiempo de proceso del módem
  vTaskDelay(pdMS_TO_TICKS(100));

  // Simulamos que el módem responde OK
  const char* response = "\r\nOK\r\n";
  parser.eatBuffer((uint8_t*)response, strlen(response));

  vTaskDelete(NULL);
}

// =========================================================
// TESTS UNITARIOS
// =========================================================

void setUp(void)
{
  spy.reset();
  // Reiniciamos el parser antes de cada test
  // Nota: UartDriver se inicializará realmente, pero no conectaremos nada a los pines
  parser.init(&driver, spyCallback, &spy);
}

void tearDown(void)
{
}

// 1. Prueba de Evento Simple (JOIN)
void test_parse_join_event(void)
{
  const char* input = "+EVT:JOINED\r\n";
  parser.eatBuffer((uint8_t*)input, strlen(input));

  TEST_ASSERT_EQUAL_STRING("JOIN", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("SUCCESS", spy.lastPayload.c_str());
}

// 2. Prueba del caso "sucio" (confirmed flag pegado al evento)
void test_parse_dirty_line_bug(void)
{
  // El caso real que fallaba antes
  const char* input = "confirmed flag: 0+EVT:RX_1, PORT 2, DR 5, RSSI -86, SNR 9\r\n";

  parser.eatBuffer((uint8_t*)input, strlen(input));

  TEST_ASSERT_EQUAL_STRING("RX_META", spy.lastType.c_str());
  // Verificamos que el payload empieza correctamente después del +EVT:
  TEST_ASSERT_TRUE(spy.lastPayload.startsWith("RX_1"));
}

// 3. Prueba del Helper de Metadatos (Static Method)
void test_metadata_helper_parsing(void)
{
  LsmRxMetadata meta;
  const char*   rawMeta = "RX_1, PORT 2, DR 5, RSSI -90, SNR 10, DMODM 10, GWN 2";

  bool result = LSM1x0A_AtParser::parseRxMetadata(rawMeta, &meta);

  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("1", meta.slot);
  TEST_ASSERT_EQUAL(2, meta.port);
  TEST_ASSERT_EQUAL(5, meta.dataRate);
  TEST_ASSERT_EQUAL(-90, meta.rssi);
  TEST_ASSERT_EQUAL(10, meta.snr);
  TEST_ASSERT_TRUE(meta.hasLinkCheck);
  TEST_ASSERT_EQUAL(10, meta.demodMargin);
  TEST_ASSERT_EQUAL(2, meta.nbGateways);
}

// 4. Prueba de Fragmentación (Buffer llega a trozos)
void test_fragmentation_handling(void)
{
  const char* part1 = "+EVT:RECV_UNCONF";
  const char* part2 = "IRMED:2:4:AABB\r\n";

  // Enviamos primera parte
  parser.eatBuffer((uint8_t*)part1, strlen(part1));
  TEST_ASSERT_EQUAL(0, spy.eventCount); // Aún no debe haber saltado

  // Enviamos segunda parte
  parser.eatBuffer((uint8_t*)part2, strlen(part2));

  TEST_ASSERT_EQUAL(1, spy.eventCount);
  TEST_ASSERT_EQUAL_STRING("RX_DATA", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("2:4:AABB", spy.lastPayload.c_str());
}

// 5. Prueba de BOOT ALERT
void test_boot_alert(void)
{
  const char* input = "\r\nBOOTALERT\r\n";
  parser.eatBuffer((uint8_t*)input, strlen(input));

  TEST_ASSERT_EQUAL_STRING("INFO", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("BOOT", spy.lastPayload.c_str());
}

// 6. Prueba de Flujo Síncrono (SendCommand)
void test_synchronous_command_ok(void)
{
  // Lanzamos una tarea que simulará ser el módem respondiendo en 100ms
  xTaskCreate(modemSimulatorTask, "ModemSim", 2048, NULL, 10, NULL);

  // Esta función bloqueará hasta que la tarea simule la respuesta
  // OJO: Usamos un timeout mayor al delay de la tarea
  AtError result = parser.sendCommand("AT", 1000);

  TEST_ASSERT_EQUAL(AtError::OK, result);
}

// 7. Prueba de Timeout
void test_synchronous_command_timeout(void)
{
  // No lanzamos simulador, así que nadie responderá
  AtError result = parser.sendCommand("AT", 200); // Timeout rápido 200ms

  TEST_ASSERT_EQUAL(AtError::TIMEOUT, result);
}

// =========================================================
// RUNNER
// =========================================================

void setup()
{
  delay(2000); // Esperar a monitor serie
  UNITY_BEGIN();

  RUN_TEST(test_parse_join_event);
  RUN_TEST(test_parse_dirty_line_bug);
  RUN_TEST(test_metadata_helper_parsing);
  RUN_TEST(test_fragmentation_handling);
  RUN_TEST(test_boot_alert);
  RUN_TEST(test_synchronous_command_ok);
  RUN_TEST(test_synchronous_command_timeout);

  UNITY_END();
}

void loop()
{
}