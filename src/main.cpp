#include "LSM1x0A_AtParser.h"
#include <Arduino.h>

UartDriver       driver;
LSM1x0A_AtParser lora;

// Callback estático para eventos (sin String)
void onLoRaEvent(const char* type, const char* payload, void* ctx)
{
  // -------------------------------------------------
  // CASO 1: METADATOS DE COBERTURA (RSSI, SNR...)
  // -------------------------------------------------
  if (strcmp(type, LsmEvent::RX_META) == 0) {
    LsmRxMetadata meta;

    // Usamos el Helper estático para parsear el string complejo
    if (LSM1x0A_AtParser::parseRxMetadata(payload, &meta)) {
      Serial.println("\n>>> [COBERTURA] Datos de Señal Recibidos <<<");
      Serial.printf("    Slot: RX_%s\n", meta.slot);
      Serial.printf("    RSSI: %d dBm\n", meta.rssi);
      Serial.printf("    SNR:  %d dB\n", meta.snr);
      Serial.printf("    DR:   %d\n", meta.dataRate);

      if (meta.hasLinkCheck) {
        Serial.printf("    [LinkCheck] Gateways: %d, Margin: %dB\n", meta.nbGateways, meta.demodMargin);
      }

      // AQUÍ: Guardar estos datos en tarjeta SD, enviarlos por WiFi, etc.
      // saveToLog(meta);
    }
    else {
      Serial.printf("[ERROR] No se pudo parsear metadata: %s\n", payload);
    }
  }

  // -------------------------------------------------
  // CASO 2: PAYLOAD DE DATOS (Lo que envió el servidor)
  // -------------------------------------------------
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    // payload formato: "PORT:SIZE:HEXDATA" (ej "2:4:AABBCCDD")
    int  port, size;
    char hexData[256];

    // Parseo simple con sscanf
    if (sscanf(payload, "%d:%d:%s", &port, &size, hexData) == 3) {
      Serial.printf("[RX DATA] Port: %d, Len: %d, Payload: %s\n", port, size, hexData);
    }
  }

  // -------------------------------------------------
  // OTROS EVENTOS
  // -------------------------------------------------
  else if (strcmp(type, LsmEvent::JOIN) == 0) {
    Serial.printf("[JOIN] %s\n", payload);
  }
  else if (strcmp(type, LsmEvent::TX) == 0) {
    Serial.printf("[TX STATUS] %s\n", payload);
  }
  else if (strcmp(type, LsmEvent::CLASS) == 0) {
    Serial.printf("[CLASE] Cambiado a Clase %s\n", payload);
  }
  else if (strcmp(type, LsmEvent::NVM) == 0) {
    Serial.println("[NVM] Datos guardados en Flash interna del módulo.");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Inicializar Parser + Driver
  if (!lora.init(&driver, onLoRaEvent)) {
    Serial.println("Fallo HW");
    while (1)
      ;
  }

  // Send ATZ to wake up
  AtError err;
  err = lora.sendCommand("ATZ", 1000);
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
  err = lora.sendCommandWithResponse("AT+DEUI=?", nullptr, devEui, sizeof(devEui));

  if (err == AtError::OK) {
    Serial.printf("Mi DevEUI es: %s\n", devEui);
  }
  else {
    Serial.printf("Error al leer EUI: %d, %s\n", (int)err, lora.atErrorToString(err));
  }

  // CASO 2: Configurar Join (Escritura simple)
  // Cambiamos las claves por las nuestras
  err = lora.sendCommand("AT+BAND=5");
  if (err != AtError::OK) {
    Serial.println("Error configurando Banda LoRaWAN.");
  }
  else {
    Serial.println("Banda LoRaWAN configurada correctamente.");
  }

  // read back to verify
  char band[64];
  err = lora.sendCommandWithResponse("AT+BAND=?", nullptr, band, sizeof(band));
  if (err == AtError::OK) {
    Serial.printf("Banda leída: %s\n", band);
  }
  else {
    Serial.printf("Error al leer Banda: %d, %s\n", (int)err, lora.atErrorToString(err));
  }

  // CASO 3: Unirse (Manejo de estados)
  //   Serial.println("Intentando Join...");
  err = lora.sendCommand("AT+JOIN=1"); // Esto solo inicia el proceso

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
  // Enviar un paquete cada 30 segundos
  static uint32_t lastTx = 0;
  if (millis() - lastTx > 30000) {
    lastTx = millis();
    Serial.println("Enviando paquete (Confirmado)...");
    // Al recibir respuesta (ACK o Downlink), saltará el evento RX_META
    lora.sendCommand("AT+SEND=33:1:0101");
  }
  delay(10);
}