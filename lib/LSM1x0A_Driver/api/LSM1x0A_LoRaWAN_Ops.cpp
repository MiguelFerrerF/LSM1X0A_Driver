#include "../LSM1x0A_Controller.h"
#include "LSM1x0A_LoRaWAN.h"
#include <Arduino.h>

// =========================================================================
// OPERACIONES DE RED LORAWAN (JOIN, SEND, LINKCHECK)
// =========================================================================

bool LSM1x0A_LoRaWAN::join(bool isOTAA, uint32_t timeoutMs)
{
  if (!_controller)
    return false;

  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::JOIN, isOTAA ? 1 : 0);

  // Limpiamos banderas anteriores
  _controller->clearEvents(LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL);

  // Enviamos el comando y esperamos el OK inicial
  AtError err = _controller->sendCommand(cmd, 2000, 1);
  if (err != AtError::OK) {
    return false;
  }

  // Ahora esperamos asíncronamente a que llegue el evento URC
  uint32_t result = _controller->waitForEvent(LSM_EVT_JOIN_SUCCESS | LSM_EVT_JOIN_FAIL, timeoutMs);

  if (result & LSM_EVT_JOIN_SUCCESS) {
    return true; // Éxito completo
  }

  // Si llegamos aquí es un fallo o Timeout inesperado de la capa MAC
  _controller->recoverModule();
  return false; // Fallo o Timeout
}

bool LSM1x0A_LoRaWAN::sendData(uint8_t port, const char* data, bool confirmed, uint32_t timeoutMs)
{
  if (!_controller)
    return false;
  if (!_controller->isJoined())
    return false;
  if (port < 1 || port > 223)
    return false;

  if (data == nullptr)
    return false;

  // AT+SEND=<Port>:<Ack>:<Payload>
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "AT+SEND=%d:%d:%s", port, confirmed ? 1 : 0, data);

  // Limpiamos banderas
  _controller->clearEvents(LSM_EVT_TX_SUCCESS | LSM_EVT_TX_FAIL | LSM_EVT_RX_TIMEOUT);

  // Enviamos el payload (esto responde OK rápido si está bien formateado y no está ocupado)
  AtError err = _controller->sendCommand(cmd, 3000, 1);
  if (err != AtError::OK) {
    return false;
  }

  // Si timeoutMs es 0 (Auto), calculamos dinámicamente en función de los reintentos
  uint32_t calcTimeoutMs = timeoutMs;

  if (confirmed) {
    if (calcTimeoutMs == 0) {
      int retries = getConfirmRetry();
      if (retries < 0)
        retries = 7; // Fallback
      // Base safety 5s + cada intento toma ~3.5 segundos de radio (Rx1+Rx2+Ack)
      calcTimeoutMs = 5000 + (retries * 3500);
    }

    // Para Confirmados, esperamos la ventana (TX Done o Rx Done) que lanza +EVT
    uint32_t result = _controller->waitForEvent(LSM_EVT_TX_SUCCESS | LSM_EVT_TX_FAIL, calcTimeoutMs);
    if (result & LSM_EVT_TX_SUCCESS) {
      return true;
    }
    
    // Si llegamos a un timeout total asíncrono, significa que el módulo dejó de responder
    _controller->recoverModule();
    return false; // Timeout o Error capa MAC
  }
  else {
    // Para Unconfirmed puro, esperamos los `MAC rxTimeOut` (2 por cada intento de transmisión)
    int retries = getUnconfirmRetry();
    if (retries < 0)
      retries = 0; // Fallback si dio error

    // (N reintentos) * 2 ventanas RX
    int expectedTimeouts = retries * 2;
    int receivedTimeouts = 0;

    // Como los reintentos pueden tardar, extendemos el timeout base por seguridad dinámica
    uint32_t dynamicTimeoutMs = 5000 + (retries * 3500);
    if (calcTimeoutMs == 0 || calcTimeoutMs < dynamicTimeoutMs) {
      calcTimeoutMs = dynamicTimeoutMs;
    }

    uint32_t startMs = millis();
    _controller->clearEvents(LSM_EVT_RX_TIMEOUT); // limpiar antes de empezar

    while ((millis() - startMs) < calcTimeoutMs) {
      // Esperamos por un timeout de RX de MAC, o si mágicamente llega un SUCCESS (por ejemplo, LinkCheck piggyback)
      uint32_t result = _controller->waitForEvent(LSM_EVT_TX_SUCCESS | LSM_EVT_RX_TIMEOUT, 3000, true);

      if (result & LSM_EVT_TX_SUCCESS)
        return true; // Downlink interceptado o MAC terminó temprano

      if (result & LSM_EVT_RX_TIMEOUT) {
        receivedTimeouts++;
        if (receivedTimeouts >= expectedTimeouts)
          return true;
      }
    }

    // Si llegamos a un timeout total asíncrono, significa que el módulo dejó de responder
    _controller->recoverModule();
    return false; // Timeout de espera global
  }
}

bool LSM1x0A_LoRaWAN::requestLinkCheck()
{
  if (!_controller)
    return false;
  if (!_controller->isJoined())
    return false;

  // AT+LINKC no genera evento inmediato, sólo agenda el request para el próximo Uplink
  AtError err = _controller->sendCommand(LsmAtCommand::LINK_CHECK, 2000, 1);
  return (err == AtError::OK);
}
