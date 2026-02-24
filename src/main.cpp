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

      if (controller->wakeUp()) {
        Serial.println("[APP] WakeUp OK.");

        // Set module to LoRaWAN mode for testing
        if (controller->setMode(LsmMode::LORAWAN)) {
          Serial.println("[APP] Modo LoRaWAN configurado OK.");
        }
        else {
          Serial.println("[APP] Error al configurar modo LoRaWAN.");
        }

        // TEST GETTERS
        Serial.println("\n--- Probando Getters ---");

        int bat = controller->getBattery();
        Serial.printf("Battery: %d mV\n", bat);

        int baud = controller->getBaudrate();
        Serial.printf("Baudrate Actual: %d\n", baud);

        char versionBuf[64];
        if (controller->getVersion(versionBuf, sizeof(versionBuf))) {
          Serial.printf("Version: %s\n", versionBuf);
        }
        else {
          Serial.println("Version: Error al obtener");
        }

        struct tm timeinfo;
        if (controller->getLocalTime(&timeinfo)) {
          Serial.printf("Local Time: %02d:%02d:%02d %02d/%02d/%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday,
                        timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        }
        else {
          Serial.println("Local Time: No disponible / Error");
        }

        LsmModuleType type = controller->getDeviceType();
        Serial.printf("Module Type: %s\n", (type == LsmModuleType::LSM100A) ? "LSM100A" : (type == LsmModuleType::LSM110A) ? "LSM110A" : "UNKNOWN");

        // TEST SETTERS
        Serial.println("\n--- Probando Setters ---");

        bool ok = controller->setVerboseLevel(2);
        Serial.printf("Set Verbose: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setDevEUI("0102030405060708");
        Serial.printf("Set DevEUI: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setAppEUI("A1B2C3D4E5F67890");
        Serial.printf("Set AppEUI: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setAppKey("0102030405060708090A0B0C0D0E0F10");
        Serial.printf("Set AppKey: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setNwkKey("00112233445566778899AABBCCDDEEFF");
        Serial.printf("Set NwkKey: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setDevAddr("26011BDA");
        Serial.printf("Set DevAddr: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setAppSKey("FFEEDDCCBBAA99887766554433221100");
        Serial.printf("Set AppSKey: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setNwkSKey("1234567890ABCDEF1234567890ABCDEF");
        Serial.printf("Set NwkSKey: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setNwkID(5);
        Serial.printf("Set NwkID: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setBand(LsmBand::EU868);
        Serial.printf("Set Band: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setClass(LsmClass::CLASS_A);
        Serial.printf("Set Class: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setChannelMask(LsmBand::EU868, 0);
        Serial.printf("Set Channel Mask SubBand 0: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setADR(false);
        Serial.printf("Set ADR: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setDataRate(LsmDataRate::DR_3);
        Serial.printf("Set DataRate: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setDutyCycle(true);
        Serial.printf("Set Duty Cycle: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setRx1Delay(1000);
        Serial.printf("Set Rx1 Delay: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setRx2Delay(2000);
        Serial.printf("Set Rx2 Delay: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setRx2DataRate(LsmDataRate::DR_2);
        Serial.printf("Set Rx2 DataRate: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setRx2Frequency(869525000);
        Serial.printf("Set Rx2 Frequency: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setJoin1Delay(500);
        Serial.printf("Set Join1 Delay: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setJoin2Delay(1000);
        Serial.printf("Set Join2 Delay: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setTxPower(LsmTxPower::TP_MAX);
        Serial.printf("Set TxPower: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setPingSlot(LsmPingSlot::EVERY_4_SEC);
        Serial.printf("Set PingSlot: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setNetworkType(LsmNetworkType::PUBLIC);
        Serial.printf("Set NetworkType: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setConfirmRetry(3);
        Serial.printf("Set ConfirmRetry: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setUnconfirmRetry(5);
        Serial.printf("Set UnconfirmRetry: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setDevNonce(42);
        Serial.printf("Set DevNonce: %s\n", ok ? "OK" : "ERROR");

        // TEST RESETS
        Serial.println("\n--- Probando Resets ---");

        Serial.println("\n[TEST] Ejecutando Software Reset (ATZ)...");
        bool srOK = controller->softwareReset();
        Serial.printf("-> Software Reset: %s\n", srOK ? "EXITOSO (Boot Alert Recibido)" : "FALLO");

        // Configurar PIN de Reset (Cambia el 15 por el pin real de tu placa si es distinto)
        int testResetPin = 15; // Ajustar pin a hardware real
        controller->setResetPin(testResetPin);

        Serial.printf("\n[TEST] Ejecutando Hardware Reset en GPIO %d...\n", testResetPin);
        bool hrOK = controller->hardwareReset();
        Serial.printf("-> Hardware Reset: %s\n", hrOK ? "EXITOSO (Boot Alert Recibido)" : "FALLO");
      }
      else {
        Serial.println("[APP] WakeUp falló.");
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