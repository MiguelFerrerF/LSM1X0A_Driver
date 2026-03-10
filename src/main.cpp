#include "LSM1x0A_Client.h"
#include <Arduino.h>

LSM1x0A_Client lsmClient;

void globalLogCallback(LsmLogLevel level, const char* component, const char* message)
{
  const char* levelStr = "UNK";
  switch (level) {
    case LsmLogLevel::ERROR:
      levelStr = "ERR";
      break;
    case LsmLogLevel::WARN:
      levelStr = "WRN";
      break;
    case LsmLogLevel::INFO:
      levelStr = "INF";
      break;
    case LsmLogLevel::DEBUG:
      levelStr = "DBG";
      break;
    case LsmLogLevel::VERBOSE:
      levelStr = "VRB";
      break;
    default:
      break;
  }
  Serial.printf("[%s][%s] %s\n", levelStr, component, message);
}

void myRxCallback(uint8_t port, const uint8_t* payload, size_t size)
{
  Serial.printf("\n--- DOWNLINK RECEIVED ---\n");
  Serial.printf("Port: %d\n", port);
  Serial.printf("Size: %d\n", size);
  Serial.print("Data (Hex): ");
  for (size_t i = 0; i < size; i++) {
    Serial.printf("%02X ", payload[i]);
  }
  Serial.println("\n-------------------------");
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- LSM1x0A Client High-Level Example ---");

  // 1. Initialize the client (starts Uart and checks connection)
  if (!lsmClient.begin(globalLogCallback, LsmLogLevel::DEBUG)) {
    Serial.println("Error: Could not communicate with LSM1x0A module.");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("Module detected and responding.");

  lsmClient.setDownlinkCallback(myRxCallback);

  // Get some Info
  char version[32];
  if (lsmClient.getFirmwareVersion(version, sizeof(version))) {
    Serial.printf("Firmware Version: %s\n", version);
  }
  Serial.printf("Battery: %d mV\n", lsmClient.getBatteryVoltage());

  // Get somo LoRaWAN info
  char devEui[32];
  if (lsmClient.getController().lorawan.getDevEUI(devEui, sizeof(devEui)))
    Serial.printf("DevEUI: %s\n", devEui);
  char appEui[32];
  if (lsmClient.getController().lorawan.getAppEUI(appEui, sizeof(appEui)))
    Serial.printf("AppEUI: %s\n", appEui);
  char appKey[64];
  if (lsmClient.getController().lorawan.getAppKey(appKey, sizeof(appKey)))
    Serial.printf("AppKey: %s\n", appKey);
  char nwkSKey[64];
  if (lsmClient.getController().lorawan.getNwkSKey(nwkSKey, sizeof(nwkSKey)))
    Serial.printf("NwkSKey: %s\n", nwkSKey);

  // 2. Setup Network
  // --- For LoRaWAN OTAA ---
  Serial.println("Setting up LoRaWAN OTAA (EU868)...");
  lsmClient.setupLoRaWAN_OTAA(LsmBand::EU868, appEui, appKey);

  // --- For Sigfox ---
  // Serial.println("Setting up Sigfox (RC1 - Europe)...");
  // lsmClient.setupSigfox(LsmRCChannel::RC1);

  // 3. Join the Network
  Serial.println("Joining network...");
  if (lsmClient.joinNetwork()) {
    Serial.println("Join procedure successful!");
  }
  else {
    Serial.println("Join failed or timed out.");
    // For Sigfox this might just mean setup failed, but usually returns true
  }
}

void loop()
{
  if (lsmClient.isJoined() == LsmJoinStatus::JOINED) {
    Serial.println("\nSending 'Hello' payload...");

    // 4. Send Data - The Client automatically routes to LoRaWAN or Sigfox!
    bool success = lsmClient.sendString("Hello", true, 33, true, 4); // Request ACK, use port 33, enable retries with max 3 attempts
    vTaskDelay(pdMS_TO_TICKS(200));                                  // Short delay to allow events to process

    if (success) {
      Serial.println("Payload sent successfully!");
      Serial.printf("Last RSSI: %d dBm\n", lsmClient.getLastRssi());
    }
    else {
      Serial.println("Failed to send payload.");
    }
  }
  else {
    Serial.println("\nNot joined. Re-trying join in 10s...");
    lsmClient.joinNetwork();
  }

  // Wait 30 seconds before next transmission
  delay(30000);
}
