#include "LSM1x0A_Controller.h"
#include <Arduino.h>

int loopCount = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- Test: Instanciacion y Destruccion del LSM1x0A_Controller ---");
  Serial.println("Comenzando en 3 segundos...");
  delay(3000);
}

void verboseEventCallback(const char* type, const char* payload, void* ctx)
{
  Serial.printf("[EVENT] Type: %s | Payload: %s\n", type, payload);
}

void loop()
{
  loopCount++;
  Serial.printf("\n--- Iteracion #%d ---\n", loopCount);

  // Imprimir free heap antes
  uint32_t heapBefore = ESP.getFreeHeap();
  Serial.printf("[MEM] Heap antes de instanciar: %lu bytes\n", heapBefore);

  {
    Serial.println("[APP] Creando controlador...");
    LSM1x0A_Controller* controller = new LSM1x0A_Controller();

    Serial.println("[APP] Inicializando controlador...");
    // Intentar init
    if (controller->begin(verboseEventCallback, nullptr)) {
      Serial.println("[APP] Controlador inicializado OK. Probando WakeUp...");

      AtError err;
      if (controller->wakeUp()) {
        Serial.println("[APP] WakeUp OK. Enviando ATZ...");
        // Enviar un comando básico síncronamente
        err = controller->sendCommand("ATZ", 3000);
        Serial.printf("[APP] Resultado de ATZ: %s\n", err == AtError::BOOT_ALERT ? "OK" : "Error");
      }
      else {
        Serial.printf("[APP] WakeUp falló. Error: %u\n", (unsigned)err);
      }
    }
    else {
      Serial.println("[APP] Error inicializando el controlador.");
    }

    Serial.println("[APP] Destruyendo controlador...");
    delete controller;
    Serial.println("[APP] Controlador destruido.");
  }

  // Imprimir free heap después
  uint32_t heapAfter = ESP.getFreeHeap();
  Serial.printf("[MEM] Heap después de destruir: %lu bytes. Diferencia: %d bytes\n", heapAfter, (int)heapAfter - (int)heapBefore);

  delay(3000); // Pausa antes de la siguiente iteración
}