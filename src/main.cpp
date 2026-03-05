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

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n--- LSM1x0A Client High-Level Example ---");

  // 1. Initialize the client (starts Uart and checks connection)
  if (!lsmClient.begin(globalLogCallback)) {
    Serial.println("Error: Could not communicate with LSM1x0A module.");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("Module detected and responding.");

  // Get some Info
  char version[32];
  if (lsmClient.getFirmwareVersion(version, sizeof(version))) {
    Serial.printf("Firmware Version: %s\n", version);
  }
  Serial.printf("Battery: %d mV\n", lsmClient.getBatteryVoltage());

  // 2. Setup Network
  // --- For LoRaWAN OTAA ---
  /*
  const char* devEui = "1122334455667788";
  const char* appEui = "0000000000000000";
  const char* appKey = "0102030405060708090A0B0C0D0E0F10";
  Serial.println("Setting up LoRaWAN OTAA (EU868)...");
  lsmClient.setupLoRaWAN_OTAA(LsmBand::EU868, devEui, appEui, appKey);
  */

  // --- For Sigfox ---
  Serial.println("Setting up Sigfox (RC1 - Europe)...");
  lsmClient.setupSigfox(LsmRCChannel::RC1);

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
  if (lsmClient.isJoined()) {
    Serial.println("\nSending 'Hello' payload...");

    // 4. Send Data - The Client automatically routes to LoRaWAN or Sigfox!
    bool success = lsmClient.sendString("Hello", false); // false = unconfirmed/no downlink

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
