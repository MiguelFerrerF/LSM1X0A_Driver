#include "LSM1x0A_Controller.h"
#include "UartDriver.h"
#include <Arduino.h>

UartDriver         driver;
LSM1x0A_Controller modem;

// Credenciales (Ejemplo)
const char* APP_EUI = "0000000000000000";
const char* APP_KEY = "11223344556677881122334455667788";

// --- Callback de Aplicación ---
// Aquí recibimos los eventos limpios del controlador
void onModemEvent(const char* type, const char* payload)
{
  if (strcmp(type, LsmEvent::LOG) == 0) {
    // payload contiene el mensaje formateado desde el controlador
    // Le ponemos una etiqueta para distinguirlo en la consola
    Serial.printf("[LSM-LOG] %s\n", payload);
    return;
  }
  if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      Serial.println(">>> APP: ¡Conectado a la Red!");
    }
    else {
      Serial.println(">>> APP: Error al unir. Reintentando...");
      // Aquí podrías activar un timer para reintentar modem.join()
    }
  }
  else if (strcmp(type, LsmEvent::TX) == 0) {
    Serial.printf(">>> APP: Transmisión finalizada (%s)\n", payload);
  }
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    Serial.printf(">>> APP: Datos recibidos: %s\n", payload);
  }
  else if (strcmp(type, LsmEvent::RX_META) == 0) {
    // Usamos el helper estático que ya teníamos para decodificar
    LsmRxMetadata meta;
    if (LSM1x0A_AtParser::parseRxMetadata(payload, &meta)) {
      Serial.printf(">>> APP: Calidad Señal -> RSSI: %d, SNR: %d\n", meta.rssi, meta.snr);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando Sistema...");

  modem.setLogLevel(LsmLogLevel::VERBOSE); // Nivel máximo de logs

  // 1. Iniciar Controlador
  // Esto inicializa driver, parser y sincroniza el 'AT' ping
  if (!modem.begin(&driver, 15, onModemEvent)) {
    Serial.println("FATAL: El módulo no responde. Revisar cableado.");
    while (1)
      ;
  }

  Serial.println("Módulo OK.");
  Serial.print("DevEUI: ");
  char devEui[32];
  modem.getDevEUI(devEui, sizeof(devEui));
  Serial.println(devEui);

  // 2. Configurar y Unirse
  modem.setBand(5); // EU868

  Serial.println("Solicitando Join...");
  modem.join();
}

void loop()
{
  // Ejemplo de envío periódico
  static uint32_t lastSend = 0;

  // Solo enviamos si estamos conectados y el módulo no está trabajando
  if (millis() - lastSend > 30000) {
    if (modem.isJoined() && !modem.isTxBusy()) {

      const char* sensorData = "0102AAFF";
      Serial.println("Enviando telemetría...");

      // Enviar al puerto 10, Confirmado = true
      if (modem.sendData(10, sensorData, true)) {
        Serial.println("Petición de envío aceptada.");
      }
      else {
        Serial.println("Error al solicitar envío (¿Busy?).");
      }

      lastSend = millis();
    }
    else {
      // Serial.println("Esperando conexión o canal libre...");
    }
  }

  delay(100);
}