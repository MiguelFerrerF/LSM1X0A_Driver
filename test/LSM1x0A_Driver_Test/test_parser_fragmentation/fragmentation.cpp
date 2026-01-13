#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include <Arduino.h>
#include <unity.h>

// =========================================================
// MOCKS Y ESPÍAS (Infraestructura de Test)
// =========================================================

UartDriver       driver;
LSM1x0A_AtParser parser;

// Estructura para registrar qué llega al callback
struct SpyContext
{
  int    eventCount;
  String lastType;
  String lastPayload;

  void reset()
  {
    eventCount  = 0;
    lastType    = "";
    lastPayload = "";
  }
};

SpyContext spy;

// Callback espía
void spyCallback(const char* type, const char* payload, void* ctx)
{
  SpyContext* s = (SpyContext*)ctx;
  s->eventCount++;
  s->lastType    = String(type);
  s->lastPayload = String(payload);
}

// Configuración inicial de cada test
void setUp(void)
{
  spy.reset();
  // Iniciamos driver con pines dummy para lógica pura
  parser.init(&driver, spyCallback, &spy);
}

void tearDown(void)
{
}

// =========================================================
// ESCENARIOS DE ESTRÉS
// =========================================================

// CASO 1: Tortura Byte a Byte
// Simula una UART muy lenta o interrumpida constantemente.
void test_fragmentation_byte_by_byte(void)
{
  // Cadena de prueba con terminador estándar
  const char* message = "+EVT:JOINED\r\n";
  size_t      len     = strlen(message);

  // Alimentamos byte a byte
  for (size_t i = 0; i < len; i++) {
    parser.eatBuffer((uint8_t*)&message[i], 1);

    char c = message[i];

    // Solo exigimos silencio (0 eventos) mientras estemos recibiendo
    // caracteres de texto útil. En cuanto llega un terminador (\r o \n),
    // permitimos que el parser procese el evento.
    if (c != '\r' && c != '\n') {
      char msg[64];
      snprintf(msg, sizeof(msg), "Fallo en indice %d ('%c'): Evento prematuro", i, c);
      TEST_ASSERT_EQUAL_INT_MESSAGE(0, spy.eventCount, msg);
    }
  }

  // Al finalizar toda la cadena, verificamos que tengamos EXACTAMENTE 1 evento.
  // Esto asegura que la secuencia \r\n no provocó un evento duplicado.
  TEST_ASSERT_EQUAL_INT(1, spy.eventCount);
  TEST_ASSERT_EQUAL_STRING("JOIN", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("SUCCESS", spy.lastPayload.c_str());
}

// CASO 2: Corte en zona crítica (+EVT:)
// Rompemos la cadena justo en medio del identificador de evento
// para asegurar que el buffer interno reconstruye bien.
void test_fragmentation_critical_split(void)
{
  // Mensaje: +EVT:RECV_CONFIRMED:2:4:CAFE\r\n
  // Lo partimos en "+EV" y "T:RECV..."

  const char* part1 = "+EV";
  const char* part2 = "T:RECV_CONFIRMED:2:4:CAFE\r\n";

  parser.eatBuffer((uint8_t*)part1, strlen(part1));
  TEST_ASSERT_EQUAL_INT(0, spy.eventCount); // Silencio...

  parser.eatBuffer((uint8_t*)part2, strlen(part2)); // Completar

  TEST_ASSERT_EQUAL_INT(1, spy.eventCount);
  TEST_ASSERT_EQUAL_STRING("RX_DATA", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("2:4:CAFE", spy.lastPayload.c_str());
}

// CASO 3: Paquetes Pegados (Batching)
// El ESP32 leyó tarde y llegaron dos líneas juntas en un solo buffer UART.
// El parser debe ser capaz de procesar ambas secuencialmente.
void test_multiple_lines_one_buffer(void)
{
  // Dos eventos pegados: Un JOINED y un RX_DATA
  const char* batch = "+EVT:JOINED\r\n+EVT:RECV_UNCONFIRMED:1:2:AB\r\n";

  parser.eatBuffer((uint8_t*)batch, strlen(batch));

  // Deberían haber saltado 2 eventos
  TEST_ASSERT_EQUAL_INT(2, spy.eventCount);

  // El último evento procesado debería ser el RX
  TEST_ASSERT_EQUAL_STRING("RX_DATA", spy.lastType.c_str());
  TEST_ASSERT_EQUAL_STRING("1:2:AB", spy.lastPayload.c_str());
}

// CASO 4: Ruido entre fragmentos
// Simula basura en la línea entre trozos válidos (muy común al arrancar)
void test_noise_between_fragments(void)
{
  const char* part1 = "+EVT:RX";
  const char* noise = "basura_debug_ignorar\r\n";
  const char* part2 = "_1, PORT 2, DR 5\r\n"; // Fin válido

  // 1. Llega la cabecera
  parser.eatBuffer((uint8_t*)part1, strlen(part1));

  // 2. OJO: Si llega ruido SIN \r\n, se pega al buffer.
  // Si el ruido TIENE \r\n, provoca un parseo fallido y limpia el buffer.
  // Probemos el caso donde el ruido rompe la línea anterior (comportamiento esperado: se pierde el evento 1)
  // O el caso donde llega ruido ANTES de un evento nuevo.

  // Vamos a probar: Ruido + Evento Valido en trozos
  parser.eatBuffer((uint8_t*)noise, strlen(noise)); // Esto limpia el buffer interno

  // Ahora enviamos el mensaje real a trozos
  parser.eatBuffer((uint8_t*)part1, strlen(part1));
  parser.eatBuffer((uint8_t*)part2, strlen(part2));

  TEST_ASSERT_EQUAL_INT(1, spy.eventCount);
  TEST_ASSERT_EQUAL_STRING("RX_META", spy.lastType.c_str());
}

// =========================================================
// RUNNER
// =========================================================

void setup()
{
  delay(2000);
  UNITY_BEGIN();

  RUN_TEST(test_fragmentation_byte_by_byte);
  RUN_TEST(test_fragmentation_critical_split);
  RUN_TEST(test_multiple_lines_one_buffer);
  RUN_TEST(test_noise_between_fragments);

  UNITY_END();
}

void loop()
{
}