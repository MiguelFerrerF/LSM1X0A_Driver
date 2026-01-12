#ifndef AT_DEFINITIONS_H
#define AT_DEFINITIONS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdbool.h>
#include <stdint.h>

// Configuración de tamaños
#define AT_MAX_LINE_SIZE 128    // Buffer interno del parser
#define AT_DEFAULT_TIMEOUT 2000 // Tiempo de espera por defecto

// Terminadores y prefijos conocidos del LSM1x0A
#define STR_AT_OK "OK"
#define STR_AT_ERROR "ERROR"
#define STR_AT_ERROR_ALT "AT_ERROR"
#define STR_AT_EVT_PREFIX "+EVT:"

// --- NUEVO: Constantes para Reset ---
#define STR_AT_BOOT_ALERT "BOOTALERT"
#define STR_AT_DEVICE_TAG "Device:" // Etiqueta para extraer Modelo

// Estados del resultado de una transacción AT
typedef enum {
  AT_RES_IDLE,
  AT_RES_PENDING,
  AT_RES_OK,
  AT_RES_ERROR,
  AT_RES_TIMEOUT,
  AT_RES_ABORTED
} ATResult_t;

// Tipos de transacciones soportadas
typedef enum {
  AT_TYPE_EXEC,    // Espera OK/ERROR
  AT_TYPE_QUERY,   // Espera DATOS + OK
  AT_TYPE_NO_RESP, // No espera nada (ciego)
  AT_TYPE_BOOT     // NUEVO: Espera flujo de texto hasta BOOTALERT
} ATCmdType_t;

// Estructura de Expectativa
typedef struct {
  ATCmdType_t type;
  char *dataBuffer;
  uint16_t bufferSize;
  SemaphoreHandle_t syncSem;
  ATResult_t status;
  volatile bool active;
} ATExpectation_t;

// Callback para eventos asíncronos
typedef void (*ATEventCallback)(const char *eventLine);

// NUEVO: Callback para controlar el pin de Reset por Hardware
// state=true (Reset Activo/Pin LOW), state=false (Reset Inactivo/Pin HIGH)
typedef void (*HwResetCallback)(bool active);

#endif // AT_DEFINITIONS_H