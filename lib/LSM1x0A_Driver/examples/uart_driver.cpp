#include "UartDriver.h"
#include <Arduino.h>

// ==========================================
// CONFIGURACIÓN (Basada en tu Test Exitoso)
// ==========================================
#define MODEM_PORT UART_NUM_2
#define MODEM_TX_PIN 33 // Pin del ESP32 que va al RX del Módulo
#define MODEM_RX_PIN 14 // Pin del ESP32 que va al TX del Módulo
#define MODEM_BAUD 9600

// Instancia global del driver
UartDriver modemDriver;

// Contexto simulado: Imagina que esto es el nombre de tu tarea o módulo
const char* moduleName = "LSM1x0A_Layer";

// ==========================================
// CALLBACK: Qué hacer cuando llegan datos
// ==========================================
// Esta función es llamada automáticamente por la tarea interna del driver
void onDataReceived(void* ctx, uint8_t* data, size_t len)
{
  // 1. Recuperamos el contexto (demostración de arquitectura)
  const char* name = (const char*)ctx;

  // 2. Imprimimos en el Monitor Serie (USB) lo que llegó del UART2
  // Usamos printf para formatear limpio
  Serial.printf("[%s] RX (%d bytes) > ", name, len);

  // Imprimir carácter a carácter para ver la respuesta ASCII
  for (size_t i = 0; i < len; i++) {
    // Si es un salto de linea, imprimimos un placeholder para que se vea en el log
    if (data[i] == '\r')
      Serial.print("\\r");
    else if (data[i] == '\n')
      Serial.print("\\n");
    else
      Serial.write((char)data[i]);
  }
  Serial.println(); // Cierre de línea visual en el monitor
}

// ==========================================
// SETUP
// ==========================================
void setup()
{
  // 1. Iniciar Debug por USB (Monitor Serie)
  Serial.begin(115200);
  // Esperar un momento a que el puerto serie estabilice
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("\n\n--- INICIANDO SISTEMA EMBEBIDO ---");

  // 2. Inicializar el Driver del Módem
  // Pasamos 'moduleName' como contexto para verificar que viaja correctamente
  bool success = modemDriver.init(MODEM_PORT, MODEM_BAUD, MODEM_RX_PIN, MODEM_TX_PIN, onDataReceived, (void*)moduleName);

  if (success) {
    Serial.println(" [OK] UartDriver inicializado.");
    Serial.printf("      Conectado en TX:%d, RX:%d\n", MODEM_TX_PIN, MODEM_RX_PIN);
  }
  else {
    Serial.println(" [ERROR] Fallo crítico al iniciar UartDriver.");
    while (1) {
      vTaskDelay(100);
    } // Bloqueo infinito
  }

  // 3. Enviar un comando AT de prueba al Módem
  const char* atCommand = "AT\r\n";
  int         bytesSent = modemDriver.sendData((const uint8_t*)atCommand, strlen(atCommand));
  Serial.printf(" [INFO] Enviados %d bytes al Módem: %s", bytesSent, atCommand);
}

// ==========================================
// LOOP PRINCIPAL (Simula la Capa de Aplicación)
// ==========================================
void loop()
{
  vTaskDelete(NULL); // Eliminamos esta tarea para que no se repita el loop
}