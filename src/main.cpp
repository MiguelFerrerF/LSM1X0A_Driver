#include "LSM1x0A_AtParser.h"
#include <Arduino.h>

UartDriver       driver;
LSM1x0A_AtParser lora;

// Callback estático para eventos (sin String)
void onLoRaEvent(const char* type, const char* payload, void* ctx)
{
  if (strcmp(type, "RX") == 0) {
    // payload es un puntero directo al buffer interno del parser.
    // Solo válido durante esta función. Si lo quieres guardar, cópialo.
    Serial.printf("[EVENTO] Dato Recibido: %s\n", payload);
  }
  else if (strcmp(type, "JOIN") == 0) {
    Serial.println("[EVENTO] ¡Join Exitoso!");
  }
  else {
    Serial.printf("[EVENTO] Tipo: %s, Payload: %s\n", type, payload);
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
  err = lora.sendCommand("ATZ\r\n");
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

  // Intentar envíar otro comando, debería fallar con BUSY
  err = lora.sendCommand("AT+DEUI=?\r\n");
  if (err == AtError::BUSY) {
    Serial.println("Correcto: Módem ocupado durante Join.");
  }
  else {
    Serial.println("Error: Se esperaba BUSY durante Join.");
  }
}

void loop()
{
  // Nada aquí. Todo funciona por interrupciones y callbacks.
  vTaskDelay(1000);
}