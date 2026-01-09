#ifndef AT_PARSER_H
#define AT_PARSER_H

#include "ATDefinitions.h"
#include <cstring>

// Forward declaration del driver serial para evitar dependencias circulares
class SerialDriver;

class ATParser {
public:
  // Constructor inyectando el driver
  ATParser();
  ~ATParser();

  // Inicialización de semáforos y buffers
  bool init();

  // Método principal: Procesa un byte entrante (Llamado desde SerialDriver
  // Task)
  void processByte(char c);

  // Configura qué esperamos recibir (Llamado por Controller antes de enviar)
  // NOTA: No bloquea aquí, solo configura.
  void setExpectation(ATCmdType_t type, char *buffer = nullptr,
                      uint16_t size = 0);

  // Obtiene el semáforo para que el Controller espere
  SemaphoreHandle_t getSemaphore();

  // Obtiene el resultado final tras el desbloqueo
  ATResult_t getStatus();

  // Fuerza la cancelación de una espera (ej. por timeout manual)
  void abort();

  // Registra la función que manejará los eventos asíncronos (+EVT)
  void registerEventCallback(ATEventCallback cb);

private:
  // Estructura interna
  char _lineBuffer[AT_MAX_LINE_SIZE]; // Buffer estático único
  uint16_t _lineIdx;                  // Índice de escritura

  ATExpectation_t _expectation;   // Estado actual de espera
  ATEventCallback _eventCallback; // Puntero a función de la App

  // Métodos internos
  void _handleLine();  // Analiza la línea completa
  void _clearBuffer(); // Resetea el buffer
};

#endif // AT_PARSER_H