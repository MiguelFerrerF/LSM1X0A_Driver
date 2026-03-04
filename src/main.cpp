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
  if (controller->begin()) {
    Serial.println("[APP] Controlador inicializado OK.");

    // Registrar el callback de Logs globales (nivel VERBOSE para maxima verbosidad)
    controller->setLogCallback(globalLogCallback, LsmLogLevel::VERBOSE);

    Serial.println("[APP] Probando WakeUp...");

    for (int i = 0; i < 5; i++) {
      if (controller->wakeUp()) {
        Serial.println("[APP] WakeUp OK.");

        // Set module to Sigfox mode for testing
        if (controller->setMode(LsmMode::SIGFOX))
          Serial.println("[APP] Modo Sigfox configurado OK.");
        else
          Serial.println("[APP] Error al configurar modo Sigfox.");

        // TEST GETTERS
        Serial.println("\n--- Probando Getters Generales ---");

        int bat = controller->getBattery();
        Serial.printf("Battery: %d mV\n", bat);

        char versionBuf[64];
        if (controller->getVersion(versionBuf, sizeof(versionBuf)))
          Serial.printf("Version: %s\n", versionBuf);
        else
          Serial.println("Version: Error al obtener");

        LsmModuleType type = controller->getDeviceType();
        Serial.printf("Module Type: %s\n", (type == LsmModuleType::LSM100A) ? "LSM100A" : (type == LsmModuleType::LSM110A) ? "LSM110A" : "UNKNOWN");

        // TEST SIGFOX GETTERS
        Serial.println("\n--- Probando Sigfox Getters ---");
        char pBuf[128];

        if (controller->sigfox.getDeviceID(pBuf, sizeof(pBuf)))
          Serial.printf("Device ID: %s\n", pBuf);
        if (controller->sigfox.getInitialPAC(pBuf, sizeof(pBuf)))
          Serial.printf("Initial PAC: %s\n", pBuf);

        Serial.printf("RC Channel: %d\n", (int)controller->sigfox.getRcChannel());
        Serial.printf("Radio Power: %d dBm\n", controller->sigfox.getRadioPower());
        Serial.printf("Public Key Mode: %s\n", controller->sigfox.getPublicKeyMode() ? "ENABLED" : "DISABLED");
        Serial.printf("Payload Encryption: %s\n", controller->sigfox.getPayloadEncryption() ? "ENABLED" : "DISABLED");

        // TEST SIGFOX SETTERS
        Serial.println("\n--- Probando Sigfox Setters ---");

        bool ok = controller->setVerboseLevel(2);
        Serial.printf("Set Verbose: %s\n", ok ? "OK" : "ERROR");

        ok = controller->sigfox.setRcChannel(LsmRCChannel::RC1); // Zone 1 (Europe)
        Serial.printf("Set RC Channel (RC1): %s\n", ok ? "OK" : "ERROR");

        ok = controller->sigfox.setRadioPower(20); // 20 dBm
        Serial.printf("Set Radio Power (20dBm): %s\n", ok ? "OK" : "ERROR");

        ok = controller->sigfox.setPublicKeyMode(false); // Enable for testing
        Serial.printf("Set Public Key Mode (FALSE): %s\n", ok ? "OK" : "ERROR");

        ok = controller->sigfox.setPayloadEncryption(false);
        Serial.printf("Set Payload Encryption (FALSE): %s\n", ok ? "OK" : "ERROR");

        // TEST SIGFOX OPERATIONS
        Serial.println("\n--- Probando Sigfox Operations ---");

        Serial.println("[TEST] Ejecutando Join (Envio de Confirmacion Downlink)...");
        ok = controller->sigfox.join();
        Serial.printf("-> Join (Downlink): %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

        if (ok) {
          int16_t   rssi = controller->sigfox.getLastRxRSSI();
          struct tm timeinfo;
          char      timeString[32] = "N/A";
          if (controller->sigfox.getLastDownlinkTime(&timeinfo)) {
            strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
          }
          Serial.printf("-> Downlink Info: RSSI = %d dBm, Time = %s\n", rssi, timeString);
        }

        Serial.println("[TEST] Ejecutando Envio de BIT (Status)...");
        ok = controller->sigfox.sendBit(true, false, 1); // Send bit '1', NO downlink, 1 repeat
        Serial.printf("-> Send BIT: %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

        Serial.println("[TEST] Ejecutando Envio de Frame (String)...");
        ok = controller->sigfox.sendString("Hello!", false, 1); // Payload, NO downlink, 1 repeat
        Serial.printf("-> Send STRING: %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

        Serial.println("[TEST] Ejecutando Envio Hexadecimal (Payload)...");
        uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
        ok                = controller->sigfox.sendPayload(payload, sizeof(payload), false, 1);
        Serial.printf("-> Send PAYLOAD: %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

        // Note: OOB and RF tests logic can be added here but they might take long or interfere with normal module loop.
        // Serial.println("[TEST] Sigfox Monarch Scan (30s)...");
        // ok = controller->sigfox.testMonarchScan(30);
        // Serial.printf("-> Monarch Scan: %s\n", ok ? "EXITOSO" : "FALLO o TIMEOUT");

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