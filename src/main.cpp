#include "LSM1x0A_AtParser.h"
#include <Arduino.h>

UartDriver       driver;
LSM1x0A_AtParser lora;

// Callback estático para eventos (sin String)
void onLoRaEvent(const char* type, const char* payload, void* ctx)
{
  // 1. EVENTO DE RECEPCIÓN (Downlink)
  if (strcmp(type, LsmEvent::RX) == 0) {
    // payload viene como "2:04:AABBCCDD" (Port:Size:Data)
    int  port, size;
    char hexData[256];

    // Parseamos la string formateada
    // Usamos sscanf con precaución o strtok (aquí sscanf para legibilidad)
    if (sscanf(payload, "%d:%d:%s", &port, &size, hexData) == 3) {
      Serial.printf("[RX] Puerto: %d, Bytes: %d, Data: %s\n", port, size, hexData);

      // Aquí podrías convertir hexData a bytes reales si necesitas
      // controlRelay(hexData);
    }
  }

  // 2. EVENTO DE JOIN
  else if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      Serial.println("[JOIN] ¡Conectado a la red LoRaWAN!");
      // Encender LED verde
    }
    else {
      Serial.println("[JOIN] Falló. Reintentando en 30s...");
      // Activar timer de reintento
    }
  }

  // 3. EVENTO DE CONFIRMACIÓN DE UPLINK
  else if (strcmp(type, LsmEvent::TX) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      Serial.println("[TX] El servidor recibió nuestro mensaje (ACK).");
    }
    else {
      Serial.println("[TX] Mensaje enviado pero NO confirmado (NACK).");
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // Inicializar Parser + Driver
  if (!lora.init(&driver, onLoRaEvent)) {
    Serial.println("Fallo HW");
    while (1)
      ;
  }

  // Send ATZ to wake up
  AtError err;
  err = lora.sendCommand("ATZ\r\n", 1000);
  if (err != AtError::BOOT_ALERT) {
    Serial.printf("Error al reiniciar módem: %d, %s\n", (int)err, lora.atErrorToString(err));
  }
  else {
    Serial.println("Módem reiniciado correctamente.");
  }

  // CASO 1: Obtener EUI (Lectura)
  // Buffer local en Stack (rápido y seguro)
  char devEui[32];

  // Pedimos AT+DEUI y esperamos que la respuesta empiece por "DevEui: "
  err = lora.sendCommandWithResponse("AT+DEUI=?\r\n", nullptr, devEui, sizeof(devEui));

  if (err == AtError::OK) {
    Serial.printf("Mi DevEUI es: %s\n", devEui);
  }
  else {
    Serial.printf("Error al leer EUI: %d, %s\n", (int)err, lora.atErrorToString(err));
  }

  // CASO 2: Configurar Join (Escritura simple)
  // El comando de AppKey es largo, pero no recibimos datos grandes, solo OK
  err = lora.sendCommand("AT+APPKEY=00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF\r\n");
  if (err != AtError::OK) {
    Serial.println("Error configurando AppKey");
  }
  else {
    Serial.println("AppKey configurada correctamente.");
  }

  // read back to verify
  char appKey[64];
  err = lora.sendCommandWithResponse("AT+APPKEY=?\r\n", nullptr, appKey, sizeof(appKey));
  if (err == AtError::OK) {
    Serial.printf("AppKey leída: %s\n", appKey);
  }
  else {
    Serial.printf("Error al leer AppKey: %d, %s\n", (int)err, lora.atErrorToString(err));
  }

  // CASO 3: Unirse (Manejo de estados)
  //   Serial.println("Intentando Join...");
  err = lora.sendCommand("AT+JOIN=1\r\n"); // Esto solo inicia el proceso

  if (err == AtError::OK) {
    Serial.println("Petición de Join enviada. Esperando evento...");
  }
  else if (err == AtError::NO_NET_JOINED) {
    Serial.println("Error: Configura las claves primero.");
  }
  else if (err == AtError::BUSY) {
    Serial.println("El módem está ocupado.");
  }
}

void loop()
{
  // Nada aquí. Todo funciona por interrupciones y callbacks.
  vTaskDelay(1000);
}