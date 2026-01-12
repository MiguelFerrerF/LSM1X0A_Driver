#include "LoRaWANController.h"
#include <Arduino.h>

// -----------------------------------------------------------------------------
// CONFIGURACIÓN DE USUARIO
// -----------------------------------------------------------------------------
#define PIN_LORA_RX 16
#define PIN_LORA_TX 17
#define PIN_LORA_RESET 4
#define LORA_BAUD_RATE 9600

// -----------------------------------------------------------------------------
// INSTANCIAS GLOBALES
// -----------------------------------------------------------------------------

// Solo instanciamos el controlador. El driver se gestiona internamente.
LoRaWANController loraController;

// -----------------------------------------------------------------------------
// CALLBACK DE RESET (Requerido por SmartReset)
// -----------------------------------------------------------------------------
void onModuleReset(bool active) {
  digitalWrite(PIN_LORA_RESET, active ? LOW : HIGH);

  if (active) {
    Serial.println("   [HW] Pin Reset -> LOW");
  } else {
    Serial.println("   [HW] Pin Reset -> HIGH");
  }

  // Pequeña espera para estabilizar el pin
  delay(100);
}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n\n=== INICIO DE SISTEMA ===");

  // 1. Configurar Pin de Reset
  pinMode(PIN_LORA_RESET, OUTPUT);
  digitalWrite(PIN_LORA_RESET, HIGH);

  // 2. Registrar el Callback de Hardware
  loraController.setHardwareResetCallback(onModuleReset);

  // 3. Inicializar Controlador (Pines + Baudrate)
  // Esto internamente arranca el driver serial y la tarea de FreeRTOS
  if (!loraController.begin(LORA_BAUD_RATE, PIN_LORA_RX, PIN_LORA_TX)) {
    Serial.println(
        "[ERROR] No se pudo iniciar el sistema LoRaWAN (Driver o RTOS).");
    while (1)
      ;
  }

  Serial.println("[INIT] Sistema iniciado. Probando conexión...");

  // 4. Ejecutar Smart Reset para verificar comunicación
  char versionBuffer[64];
  memset(versionBuffer, 0, sizeof(versionBuffer));

  bool success =
      loraController.smartReset(versionBuffer, sizeof(versionBuffer));

  // 5. Mostrar Resultados
  if (success) {
    Serial.println("\n--- CONEXION EXITOSA ---");
    if (strlen(versionBuffer) > 0) {
      Serial.printf("Modelo: %s\n", versionBuffer);
    } else {
      Serial.println("Modelo: Detectado (Info no disponible)");
    }
  } else {
    Serial.println("\n--- ERROR CRITICO ---");
    Serial.println("El módulo no responde a comandos ATZ ni al reset físico.");
  }
}

void loop() { delay(1000); }