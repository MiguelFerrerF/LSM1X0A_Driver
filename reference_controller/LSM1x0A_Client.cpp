#include "LSM1x0A_Client.h"
#include <Arduino.h>

LSM1x0A_Client* LSM1x0A_Client::instance = nullptr;

void LSM1x0A_Client::updateState(ClientState newState)
{
  state = newState;
}

void LSM1x0A_Client::logDebug(const char* message)
{
  if (debugCallback) {
    debugCallback(message);
  }
  Serial.printf("%s\n", message);
}

void LSM1x0A_Client::onModemEvent(const char* type, const char* payload)
{
  if (instance == nullptr) {
    instance->logDebug(">>> APP: Evento del modem recibido pero la instancia del cliente no está inicializada.");
    return;
  }

  // 1. LOGS: Solo para depuración (Monitor Serie)
  if (strcmp(type, LsmEvent::LOG) == 0) {
    instance->logDebug(payload);
    return;
  }

  // 2. JOIN: Estado de conexión
  else if (strcmp(type, LsmEvent::JOIN) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) { // O usar LsmStatus::SUCCESS si añadiste el namespace
      instance->logDebug(">>> APP: Red Unida Correctamente");
      instance->updateState(ClientState::NETWORK_READY);
      instance->statusJoin = JOINED;
    }
    else if (strcmp(payload, "REQUIRED") == 0) {
      instance->logDebug(">>> APP: Unión requerida. Por favor, inicia el proceso de unión desde la app.");
      instance->updateState(ClientState::IDLE); // Volver a IDLE para permitir que la app inicie el proceso de unión
      instance->statusJoin = NOT_JOINED;
    }
    else {
      instance->logDebug(">>> APP: Fallo al unir. (Ver logs para motivo)");
      // Aquí podrías encender un LED rojo
      instance->updateState(ClientState::IDLE); // Volver a IDLE para permitir reintentos manuales desde la app
      instance->statusJoin = NOT_JOINED;
    }
  }

  // 3. TX: Confirmación de envío
  else if (strcmp(type, LsmEvent::TX) == 0) {
    if (strcmp(payload, "SUCCESS") == 0) {
      instance->logDebug(">>> APP: Mensaje enviado OK");
      instance->statusJoin    = JOINED;
      instance->sendFailCount = 0; // Resetear contador de fallos
    }
    else if (strcmp(payload, "NOT JOINED") == 0) {
      Serial.println(">>> APP: No unido a la red, no se puede enviar.");
      instance->updateState(ClientState::IDLE); // Volver a IDLE para permitir que la app inicie el proceso de unión
      instance->statusJoin = NOT_JOINED;
    }
    else if (strcmp(payload, "FAILED") == 0) {
      instance->logDebug(">>> APP: Error enviando mensaje");
      instance->sendFailCount++;
      if (instance->sendFailCount > 5) {
        instance->logDebug(">>> APP: Múltiples fallos de envío");
        instance->statusJoin    = NOT_JOINED;
        instance->sendFailCount = 0;
      }
      else {
        instance->logDebug(">>> APP: Reintenta enviar el último mensaje...");
        instance->state = ClientState::NEED_RETRY;
        return;
      }
    }
    else if (strcmp(payload, "BUSY") == 0) {
      instance->logDebug(">>> APP: Módulo ocupado, no se puede transmitir ahora.");
      instance->sendFailCount++;
      if (instance->sendFailCount > 5) {
        instance->logDebug(">>> APP: Múltiples intentos con módulo ocupado");
        instance->statusJoin    = NOT_JOINED;
        instance->sendFailCount = 0;
      }
      else {
        instance->logDebug(">>> APP: Reintenta enviar el último mensaje cuando el módulo esté libre...");
        instance->state = ClientState::NEED_RETRY;
        return; // Salir sin actualizar a IDLE para permitir el reintento automático
      }
    }
    instance->updateState(ClientState::IDLE); // Volver a estado listo para enviar más datos
  }

  // 4. RX_META: Calidad de señal (Funciona igual para LoRa y Sigfox ahora)
  else if (strcmp(type, LsmEvent::RX_META) == 0) {
    LsmRxMetadata meta;
    // Como formateamos el payload de Sigfox en el controller, este parser ahora sirve para ambos
    if (LSM1x0A_AtParser::parseRxMetadata(payload, &meta)) {
      char buffer[128];
      snprintf(buffer, sizeof(buffer), ">>> APP: Calidad Señal [RSSI: %d dBm] [SNR: %d]", meta.rssi, meta.snr);
      instance->logDebug(buffer);
      instance->statusJoin = JOINED;
    }
    instance->updateState(ClientState::IDLE); // Volver a estado listo para recibir más datos
  }

  // 5. RX_DATA: Datos puros
  else if (strcmp(type, LsmEvent::RX_DATA) == 0) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), ">>> APP: Payload recibido: %s", payload);
    instance->logDebug(buffer);
    // Convertir el payload hexadecimal a bytes y llamar al callback del usuario
    instance->handleDownlink(payload);
    instance->updateState(ClientState::IDLE); // Volver a estado listo para recibir
    instance->statusJoin = JOINED;
  }

  // 6. INFO u otros eventos
  else if (strcmp(type, LsmEvent::INFO) == 0) {
    if (strcmp(payload, "BOOT") == 0) {
      instance->logDebug(">>> APP: Módulo reiniciado");
      instance->updateState(ClientState::IDLE);
      instance->statusJoin    = NOT_JOINED; // Resetear estado de unión
      instance->sendFailCount = 0;          // Resetear contador de fallos
    }
  }
  else {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), ">>> APP: Otro evento [%s]: %s", type, payload);
    instance->logDebug(buffer);
  }
}

void LSM1x0A_Client::handleDownlink(const char* payload)
{
  if (dataCallback == nullptr) {
    instance->logDebug(">>> APP: No se ha definido un callback para datos entrantes.");
    return;
  }

  char hexData[128] = {0};
  int  port = 0, nbytes = 0;
  int  matched = sscanf(payload, "%d:%d:%127s", &port, &nbytes, hexData);
  if (matched != 3) {
    instance->logDebug(">>> APP: Payload recibido sin formato esperado (sscanf falló).");
    return;
  }

  // Convertir el string hexadecimal a bytes
  size_t len = strlen(hexData);
  if (len % 2 != 0) {
    instance->logDebug(">>> APP: Payload hexadecimal inválido (longitud impar).");
    return;
  }

  size_t   byteLen = len / 2;
  uint8_t* data    = new uint8_t[byteLen];
  for (size_t i = 0; i < byteLen; ++i) {
    sscanf(hexData + i * 2, "%2hhx", &data[i]);
  }

  // Llamar al callback del usuario con los datos binarios
  dataCallback(data, byteLen);

  delete[] data;
}

LSM1x0A_Client::LSM1x0A_Client()
{
  instance      = this; // Para que el callback estático pueda acceder a esta instancia
  controller    = nullptr;
  uartDriver    = nullptr;
  dataCallback  = nullptr;
  debugCallback = nullptr;
  state         = ClientState::IDLE;
}

LSM1x0A_Client::~LSM1x0A_Client()
{
  if (controller) {
    delete controller;
    controller = nullptr;
  }
  if (uartDriver) {
    delete uartDriver;
    uartDriver = nullptr;
  }
}

void LSM1x0A_Client::begin()
{
  if (uartDriver != nullptr || controller != nullptr) {
    logDebug(">>> APP: El cliente ya ha sido inicializado.");
    return;
  }

  uartDriver = new UartDriver();
  controller = new LSM1x0A_Controller();

  controller->setLogLevel(LsmLogLevel::INFO);

  if (!controller->begin(uartDriver, LSM1X0A_RESET_PIN, onModemEvent)) {
    updateState(ClientState::ERROR);
    logDebug(">>> APP: Error inicializando el controlador LSM1x0A");
    return;
  }

  if (!controller->setMode(LSM1X0A_LORA_MODE)) {
    logDebug(">>> APP: Error configurando el modo LoRa");
    updateState(ClientState::ERROR);
    return;
  }
}

void LSM1x0A_Client::begin(LoRaDataCallback downlinkCallback, LoRaDebugCallback userCallback)
{
  dataCallback  = downlinkCallback;
  debugCallback = userCallback;
  begin();
}

void LSM1x0A_Client::defaultConfig(uint8_t band, uint8_t power, bool adr, uint8_t dr, bool ack)
{
  controller->setBand(static_cast<LsmBand>(band));
  controller->setTxPower(static_cast<LsmTxPower>(power));
  controller->setClass(LsmClass::CLASS_A); // Para evitar congestión en la red.
  controller->setADR(adr);
  if (!adr)
    controller->setDataRate(static_cast<LsmDataRate>(dr));
  isAck = ack;
}

void LSM1x0A_Client::joinNetwork(LsmJoinMode mode)
{
  updateState(ClientState::JOINING);
  bool joinResult = controller->lora_join(mode == LsmJoinMode::OTAA);
  if (!joinResult) {
    if (debugCallback) {
      debugCallback("Error iniciando proceso de unión");
    }
    updateState(ClientState::ERROR);
    statusJoin = NOT_JOINED;
    return;
  }
  statusJoin = JOIN_IN_PROCESS;
}

void LSM1x0A_Client::sendData(LoRaData data)
{
  if (state == ClientState::IDLE || state == ClientState::NETWORK_READY) {
    logDebug(">>> APP: Iniciando envío de datos...");
    lastSentData = data; // Guardar el último mensaje enviado para posibles reintentos
  }
  else if (state == ClientState::NEED_RETRY) {
    logDebug(">>> APP: Reintentando enviar el último mensaje...");
  }
  else {
    logDebug(">>> APP: No se puede enviar. El módulo no está listo.");
    return;
  }

  updateState(ClientState::TRANSMITTING);

  char hexPayload[161]; // Cada byte se representa con 2 caracteres hex + null terminator
  lastSentData.toHexString(hexPayload, sizeof(hexPayload));

  bool sendResult = controller->lora_sendData(lwPort, hexPayload, isAck);
  if (!sendResult) {
    logDebug(">>> APP: Error iniciando transmisión.");
    updateState(ClientState::IDLE); // Volver a IDLE para permitir reintentos
  }
}

bool LSM1x0A_Client::recoverModule()
{
  if (controller) {
    logDebug(">>> APP: Intentando recuperación del módulo...");
    if (controller->recoverModule()) {
      logDebug(">>> APP: Módulo recuperado exitosamente.");
      updateState(ClientState::IDLE);
      statusJoin = NOT_JOINED; // Resetear estado de unión
      return true;
    }
    else {
      logDebug(">>> APP: Fallo en la recuperación del módulo.");
      updateState(ClientState::ERROR);
      return false;
    }
  }
  return false;
}

void LSM1x0A_Client::setDataCallback(LoRaDataCallback dataCallback)
{
  this->dataCallback = dataCallback;
}

void LSM1x0A_Client::setAppEUI(const unsigned char appEUIBytes[8])
{
  // Example :  01:01:01:01:01:01:01:01
  char appEUIString[24]; // 16 chars + null terminator
  for (int i = 0; i < 7; ++i) {
    sprintf(appEUIString + i * 3, "%02X:", appEUIBytes[i]);
  }
  sprintf(appEUIString + 7 * 3, "%02X", appEUIBytes[7]); // Last byte without colon
  appEUIString[23] = '\0';                               // Null-terminate the string
  controller->setAppEUI(appEUIString);
}

void LSM1x0A_Client::setAppKey(const unsigned char appKeyBytes[16])
{
  // Example : 2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C
  char appKeyString[48]; // 32 chars + null terminator
  for (int i = 0; i < 15; ++i) {
    sprintf(appKeyString + i * 3, "%02X:", appKeyBytes[i]);
  }
  sprintf(appKeyString + 15 * 3, "%02X", appKeyBytes[15]); // Last byte without colon
  appKeyString[47] = '\0';                                 // Null-terminate the string
  controller->setAppKey(appKeyString);
  controller->setNwkKey(appKeyString); // Para LoRaWAN 1.1, el AppKey se usa también como NwkKey si no se especifica otro valor
}

void LSM1x0A_Client::setADR(const bool adr)
{
  controller->setADR(adr);
}

void LSM1x0A_Client::setDR(const uint8_t dr)
{
  controller->setDataRate(static_cast<LsmDataRate>(dr));
}

void LSM1x0A_Client::setClass(const char LoRaClass)
{
  if (LoRaClass < 'A' || LoRaClass > 'C') {
    logDebug(">>> APP: Clase LoRa inválida. Use 'A', 'B' o 'C'.");
    return;
  }
  controller->setClass(static_cast<LsmClass>(LoRaClass - 'A')); // Convert 'A', 'B', 'C' to 0, 1, 2
}

void LSM1x0A_Client::setPower(const uint8_t power)
{
  controller->setTxPower(static_cast<LsmTxPower>(power));
}

void LSM1x0A_Client::setDevAddr(const unsigned char devAddrBytes[4])
{
  // Example : 26:01:1B:FF
  char devAddrString[12]; // 8 chars + null terminator
  for (int i = 0; i < 3; ++i) {
    sprintf(devAddrString + i * 3, "%02X:", devAddrBytes[i]);
  }
  sprintf(devAddrString + 3 * 3, "%02X", devAddrBytes[3]); // Last byte without colon
  devAddrString[11] = '\0';                                // Null-terminate the string
  controller->setDevAddr(devAddrString);
}

void LSM1x0A_Client::setNetID(const uint8_t id)
{
  controller->setNwkID(String(id).c_str());
}

void LSM1x0A_Client::setBand(uint8_t band)
{
  controller->setBand(static_cast<LsmBand>(band));
}

bool LSM1x0A_Client::setDevNonce(uint16_t nonce)
{
  return controller->setDevNonce(nonce);
}

void LSM1x0A_Client::setPort(const uint8_t port)
{
  lwPort = port;
}

void LSM1x0A_Client::setAck(const bool ack)
{
  isAck = ack;
}

void LSM1x0A_Client::setRetransmission(const uint8_t retransmissions)
{
  controller->setConfirmRetry(retransmissions);
  controller->setUnconfirmedRetry(retransmissions);
}

void LSM1x0A_Client::setChannelMask(uint8_t channel)
{
  controller->setChannelMaskBySubBand(static_cast<LsmBand>(controller->getBand()), channel);
}

void LSM1x0A_Client::setDutyCycle(const bool dutyCycle)
{
  controller->setDutyCycle(dutyCycle);
}

void LSM1x0A_Client::setLinkCheck()
{
  controller->setLinkCheck();
}

bool LSM1x0A_Client::getModuleType()
{
  return controller->isLSM110A();
}

ClientState LSM1x0A_Client::getState()
{
  return state;
}

void LSM1x0A_Client::getModuleVersion(char* outBuffer, size_t outSize)
{
  controller->getVersion(outBuffer, outSize);
}

void LSM1x0A_Client::getDevEUI(unsigned char devEUIBytes[8])
{
  char devEUIString[24]; // 16 chars + null terminator
  controller->getDevEUI(devEUIString, sizeof(devEUIString));
  sscanf(devEUIString, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &devEUIBytes[0], &devEUIBytes[1], &devEUIBytes[2], &devEUIBytes[3],
         &devEUIBytes[4], &devEUIBytes[5], &devEUIBytes[6], &devEUIBytes[7]);
}

void LSM1x0A_Client::getAppEUI(unsigned char appEUIBytes[8])
{
  char appEUIString[24]; // 16 chars + null terminator
  controller->getAppEUI(appEUIString, sizeof(appEUIString));
  sscanf(appEUIString, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", &appEUIBytes[0], &appEUIBytes[1], &appEUIBytes[2], &appEUIBytes[3],
         &appEUIBytes[4], &appEUIBytes[5], &appEUIBytes[6], &appEUIBytes[7]);
}

void LSM1x0A_Client::getAppKey(unsigned char appKeyBytes[16])
{
  char appKeyString[48]; // 32 chars + null terminator
  controller->getAppKey(appKeyString, sizeof(appKeyString));
  sscanf(appKeyString, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
         &appKeyBytes[0], &appKeyBytes[1], &appKeyBytes[2], &appKeyBytes[3], &appKeyBytes[4], &appKeyBytes[5], &appKeyBytes[6], &appKeyBytes[7],
         &appKeyBytes[8], &appKeyBytes[9], &appKeyBytes[10], &appKeyBytes[11], &appKeyBytes[12], &appKeyBytes[13], &appKeyBytes[14],
         &appKeyBytes[15]);
}

bool LSM1x0A_Client::getADR()
{
  return controller->getADR();
}

uint8_t LSM1x0A_Client::getDR()
{
  return static_cast<uint8_t>(controller->getDataRate());
}

char LSM1x0A_Client::getClass()
{
  return 'A' + static_cast<char>(controller->getClass()); // Convert 0,1,2 to 'A','B','C'
}

uint8_t LSM1x0A_Client::getPower()
{
  return static_cast<uint8_t>(controller->getTxPower());
}

void LSM1x0A_Client::getDevAddr(unsigned char devAddrBytes[4])
{
  char devAddrString[12]; // 8 chars + null terminator
  controller->getDevAddr(devAddrString, sizeof(devAddrString));
  sscanf(devAddrString, "%02hhX:%02hhX:%02hhX:%02hhX", &devAddrBytes[0], &devAddrBytes[1], &devAddrBytes[2], &devAddrBytes[3]);
}

uint8_t LSM1x0A_Client::getNetID()
{
  char nwkIDString[12]; // 8 chars + null terminator
  controller->getNwkID(nwkIDString, sizeof(nwkIDString));
  return static_cast<uint8_t>(atoi(nwkIDString));
}

uint8_t LSM1x0A_Client::getPort()
{
  return lwPort;
}

uint8_t LSM1x0A_Client::getJoinStatus()
{
  return statusJoin;
}

uint8_t LSM1x0A_Client::getRetransmission()
{
  return controller->getConfirmRetry();
}

uint8_t LSM1x0A_Client::getBand()
{
  return static_cast<uint8_t>(controller->getBand());
}

bool LSM1x0A_Client::getAck()
{
  return isAck;
}

uint8_t LSM1x0A_Client::getChannelMask()
{
  char buffer[32] = {0};
  controller->getChannelMask(buffer, sizeof(buffer));
  return 0; // Placeholder, implementa el getter real en el controlador y retorna su valor aquí
}

bool LSM1x0A_Client::getDutyCycle()
{
  return controller->getDutyCycle();
}
