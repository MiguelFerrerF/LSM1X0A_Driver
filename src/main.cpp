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

  Serial.println("[APP] Creando controlador...");
  LSM1x0A_Controller* controller = new LSM1x0A_Controller();

  Serial.println("[APP] Inicializando controlador...");
  // Intentar init
  if (controller->begin(verboseEventCallback, nullptr)) {
    Serial.println("[APP] Controlador inicializado OK. Probando WakeUp...");

    for (int i = 0; i < 5; i++) {
      if (controller->wakeUp()) {
        Serial.println("[APP] WakeUp OK.");

        // Set module to LoRaWAN mode for testing
        if (controller->setMode(LsmMode::LORAWAN))
          Serial.println("[APP] Modo LoRaWAN configurado OK.");
        else
          Serial.println("[APP] Error al configurar modo LoRaWAN.");

        // TEST GETTERS
        Serial.println("\n--- Probando Getters ---");

        int bat = controller->getBattery();
        Serial.printf("Battery: %d mV\n", bat);

        int baud = controller->getBaudrate();
        Serial.printf("Baudrate Actual: %d\n", baud);

        char versionBuf[64];
        if (controller->getVersion(versionBuf, sizeof(versionBuf)))
          Serial.printf("Version: %s\n", versionBuf);
        else
          Serial.println("Version: Error al obtener");

        struct tm timeinfo;
        if (controller->getLocalTime(&timeinfo))
          Serial.printf("Local Time: %02d:%02d:%02d %02d/%02d/%04d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday,
                        timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        else
          Serial.println("Local Time: No disponible / Error");

        LsmModuleType type = controller->getDeviceType();
        Serial.printf("Module Type: %s\n", (type == LsmModuleType::LSM100A) ? "LSM100A" : (type == LsmModuleType::LSM110A) ? "LSM110A" : "UNKNOWN");

        // TEST LORAWAN GETTERS
        Serial.println("\n--- Probando LoRaWAN Getters ---");
        char pBuf[128];

        if (controller->lorawan.getDevEUI(pBuf, sizeof(pBuf)))
          Serial.printf("DevEUI: %s\n", pBuf);
        if (controller->lorawan.getAppEUI(pBuf, sizeof(pBuf)))
          Serial.printf("AppEUI: %s\n", pBuf);
        if (controller->lorawan.getAppKey(pBuf, sizeof(pBuf)))
          Serial.printf("AppKey: %s\n", pBuf);
        if (controller->lorawan.getNwkKey(pBuf, sizeof(pBuf)))
          Serial.printf("NwkKey: %s\n", pBuf);
        if (controller->lorawan.getDevAddr(pBuf, sizeof(pBuf)))
          Serial.printf("DevAddr: %s\n", pBuf);
        if (controller->lorawan.getAppSKey(pBuf, sizeof(pBuf)))
          Serial.printf("AppSKey: %s\n", pBuf);
        if (controller->lorawan.getNwkSKey(pBuf, sizeof(pBuf)))
          Serial.printf("NwkSKey: %s\n", pBuf);

        Serial.printf("NwkID: %d\n", controller->lorawan.getNwkID());
        Serial.printf("DevNonce: %d\n", controller->lorawan.getDevNonce());
        Serial.printf("ADR: %d\n", controller->lorawan.getADR());
        Serial.printf("DataRate: %d\n", (int)controller->lorawan.getDataRate());
        Serial.printf("TxPower: %d\n", (int)controller->lorawan.getTxPower());
        Serial.printf("Band: %d\n", (int)controller->lorawan.getBand());
        Serial.printf("Class: %d\n", (int)controller->lorawan.getClass());
        Serial.printf("DutyCycle: %d\n", controller->lorawan.getDutyCycle());
        Serial.printf("Join1Delay: %d\n", controller->lorawan.getJoin1Delay());
        Serial.printf("Join2Delay: %d\n", controller->lorawan.getJoin2Delay());
        Serial.printf("Rx1Delay: %d\n", controller->lorawan.getRx1Delay());
        Serial.printf("Rx2Delay: %d\n", controller->lorawan.getRx2Delay());
        Serial.printf("Rx2DataRate: %d\n", (int)controller->lorawan.getRx2DataRate());
        Serial.printf("Rx2Frequency: %ld\n", controller->lorawan.getRx2Frequency());
        Serial.printf("PingSlot: %d\n", (int)controller->lorawan.getPingSlot());
        Serial.printf("ConfirmRetry: %d\n", controller->lorawan.getConfirmRetry());
        Serial.printf("UnconfirmRetry: %d\n", controller->lorawan.getUnconfirmRetry());
        Serial.printf("NetworkType: %d\n", (int)controller->lorawan.getNetworkType());
        Serial.printf("DevNonce: %d\n", controller->lorawan.getDevNonce());

        // TEST SETTERS
        Serial.println("\n--- Probando Setters ---");

        bool ok = controller->setVerboseLevel(2);
        Serial.printf("Set Verbose: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setDevEUI("0102030405060708");
        // Serial.printf("Set DevEUI: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setAppEUI("A1B2C3D4E5F67890");
        // Serial.printf("Set AppEUI: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setAppKey("0102030405060708090A0B0C0D0E0F10");
        // Serial.printf("Set AppKey: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setNwkKey("0102030405060708090A0B0C0D0E0F10");
        // Serial.printf("Set NwkKey: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setDevAddr("26011BDA");
        // Serial.printf("Set DevAddr: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setAppSKey("0102030405060708090A0B0C0D0E0F10");
        // Serial.printf("Set AppSKey: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setNwkSKey("0102030405060708090A0B0C0D0E0F10");
        // Serial.printf("Set NwkSKey: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setNwkID(5);
        // Serial.printf("Set NwkID: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setBand(LsmBand::EU868);
        Serial.printf("Set Band: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setClass(LsmClass::CLASS_A);
        Serial.printf("Set Class: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setChannelMask(LsmBand::EU868);
        Serial.printf("Set Channel Mask SubBand 0: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setADR(true);
        Serial.printf("Set ADR: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setDataRate(LsmDataRate::DR_3);
        // Serial.printf("Set DataRate: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setDutyCycle(true);
        // Serial.printf("Set Duty Cycle: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setRx1Delay(1000);
        // Serial.printf("Set Rx1 Delay: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setRx2Delay(2000);
        // Serial.printf("Set Rx2 Delay: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setRx2DataRate(LsmDataRate::DR_2);
        // Serial.printf("Set Rx2 DataRate: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setRx2Frequency(869525000);
        // Serial.printf("Set Rx2 Frequency: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setJoin1Delay(500);
        // Serial.printf("Set Join1 Delay: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setJoin2Delay(1000);
        // Serial.printf("Set Join2 Delay: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setTxPower(LsmTxPower::TP_MAX);
        Serial.printf("Set TxPower: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setPingSlot(LsmPingSlot::EVERY_4_SEC);
        // Serial.printf("Set PingSlot: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setNetworkType(LsmNetworkType::PUBLIC);
        Serial.printf("Set NetworkType: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setConfirmRetry(3);
        Serial.printf("Set ConfirmRetry: %s\n", ok ? "OK" : "ERROR");

        ok = controller->lorawan.setUnconfirmRetry(5);
        Serial.printf("Set UnconfirmRetry: %s\n", ok ? "OK" : "ERROR");

        // ok = controller->lorawan.setDevNonce(42);
        // Serial.printf("Set DevNonce: %s\n", ok ? "OK" : "ERROR");

        // TEST LORAWAN OPERATIONS (ASYNCHRONOUS SIMULATED AS SYNCHRONOUS)
        Serial.println("\n--- Probando LoRaWAN Operations ---");

        Serial.println("[TEST] Ejecutando Join (OTAA) con rcon un máximo de 5 intentos...");
        for (int i = 1; i <= 5; i++) {
          Serial.printf("-> Intento de Join %d/5...\n", i);
          ok = controller->lorawan.join(LsmJoinMode::OTAA);
          if (ok) {
            Serial.println("-> Join (OTAA): EXITOSO");
            delay(500);
            break;
          }
          else {
            Serial.println("-> Join (OTAA): FALLO o TIMEOUT");
            if (i < 5)
              delay(5000); // Esperar 5s antes de reintentar
          }
        }

        if (controller->lorawan.isJoined()) {
          Serial.println("[TEST] Ejecutando Send Data (Con Confirmacion) en Puerto 2...");
          ok = controller->lorawan.sendData(2, "01020304AA", true);
          Serial.printf("-> Send Data (Confirmado): %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

          // Serial.println("[TEST] Solicitando LinkCheck para el proximo uplink...");
          // ok = controller->lorawan.requestLinkCheck();
          // Serial.printf("-> Request LinkCheck: %s\n", ok ? "OK" : "ERROR (Not Joined o Busy)");

          Serial.println("[TEST] Ejecutando Send Data (Sin Confirmacion) en Puerto 3 (5 Mensajes)...");
          for (int i = 1; i <= 5; i++) {
            Serial.printf("\n-> Enviando Mensaje Unconfirmed %d/5...\n", i);
            ok = controller->lorawan.sendData(3, "BBCCDDEE", false);
            Serial.printf("-> Resultado: %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");
            
            // Imprimir metadatos guardados si el envío generó recepción RX_META o info de LinkCheck en RX2
            Serial.printf("   [Metadatos] RSSI: %d, SNR: %d, DMODM: %d, GWN: %d\n", 
              controller->getLastRssi(), controller->getLastSnr(), 
              controller->getLastDemodMargin(), controller->getLastNbGateways());
              
            delay(3000); // Pequeña pausa entre envíos
          }
        }
        else {
          Serial.println("[TEST] Ignorando Send Data porque el Join ha fallado.");
        }

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

        break;
      }
      else {
        Serial.println("[APP] WakeUp falló.");
        if (controller->recoverModule()) {
          Serial.println("[APP] Recover OK.");
          break;
        }
        else
          Serial.println("[APP] Recover falló.");
      }
    }
  }
  else {
    Serial.println("[APP] Error inicializando el controlador.");
  }

  Serial.println("[APP] Destruyendo controlador...");
  delete controller;
  Serial.println("[APP] Controlador destruido.");

  // Imprimir free heap después
  uint32_t heapAfter = ESP.getFreeHeap();
  Serial.printf("[MEM] Heap después de destruir: %lu bytes. Diferencia: %d bytes\n", heapAfter, (int)heapAfter - (int)heapBefore);

  delay(3000); // Pausa antes de la siguiente iteración
}