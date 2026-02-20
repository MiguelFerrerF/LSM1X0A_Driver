#ifndef LSM1X0A_CONTROLLER_H
#define LSM1X0A_CONTROLLER_H

#include "LSM1x0A_AtParser.h"
#include "UartDriver.h"
#include <Arduino.h>

/**
 * @class LSM1x0A_Controller
 * @brief Controlador intuitivo para interactuar con módulos LSM1x0A (LoRaWAN/Sigfox).
 * 
 * Esta clase proporciona una interfaz sencilla para el usuario, abstrayendo 
 * el driver UART y el parseador AT subyacentes. Permite inicializar la 
 * comunicación, gestionar callbacks de eventos y enviar comandos generales 
 * obteniendo sus respuestas fácilmente.
 */
class LSM1x0A_Controller {
public:
    /**
     * @brief Constructor por defecto.
     * Crea las instancias internas de UartDriver y LSM1x0A_AtParser, 
     * listas para ser inicializadas mediante begin().
     */
    LSM1x0A_Controller();

    /**
     * @brief Destructor.
     * Libera recursos y de-inicializa los componentes internos.
     */
    ~LSM1x0A_Controller();

    /**
     * @brief Inicializa el hardware UART y el Parser interno.
     * 
     * @param callback Función de callback (ej. onLoRaEvent) que el usuario 
     * defina para procesar eventos asíncronos (+EVT:...).
     * @param ctx Un puntero de contexto opcional para el callback (puede ser this o nullptr).
     * @return true si la UART se inició con éxito y la memoria fue alocada, false en otro caso.
     */
    bool begin(AtEventCallback callback, void* ctx = nullptr);

    /**
     * @brief Detiene el hardware y desvincula los event listeners.
     */
    void end();

    /**
     * @brief Despierta al módulo e intenta sincronizar su estado inicial.
     * Envía comandos AT o caracteres 'Wake-Up' básicos.
     * 
     * @return true si el módulo responde "OK" a la orden básica.
     */
    bool wakeUp();

    /**
     * @brief Envía cualquier comando AT al módulo, devolviendo error si falla.
     * Utiliza internamente la lógica del LsmAtParser.
     * 
     * @param cmd El comando AT exacto (ej. "ATZ" o "AT+MODE=1").
     * @param timeoutMs Tiempo máximo bloqueante de espera (por defecto 2000 ms).
     * @return AtError::OK si el módulo respondió OK de forma síncrona, otro Enum si no.
     */
    AtError sendCommand(const char* cmd, uint32_t timeoutMs = 2000);

    /**
     * @brief Envía un comando AT y captura su respuesta en un bloque de texto.
     * Ideal para getters (ej. "AT+DEUI=?", "AT+BAT=?").
     * 
     * @param cmd El comando AT exacto.
     * @param outBuffer El buffer donde el usuario quiere guardar la respuesta (sin el comando de eco o el OK).
     * @param outSize Capacidad del outBuffer.
     * @param expectedTag Si se espera que el módulo prefije la respuesta con un tag (ej. "APP_VERSION:" o "DevEui:"), esto lo filtra. Usa nullptr para cadena en bruto.
     * @param timeoutMs Tiempo de espera.
     * @return AtError::OK si se completó y se copió algo en outBuffer.
     */
    AtError sendCommandWithResponse(
        const char* cmd, 
        char* outBuffer, 
        size_t outSize, 
        const char* expectedTag = nullptr, 
        uint32_t timeoutMs = 2000
    );

    // =========================================================================
    // COMANDOS AT BÁSICOS / GENERALES
    // =========================================================================
    
    /**
     * @brief Obtiene el voltaje de la batería en mV.
     * @return El voltaje >= 0 (ej. 3300 para 3.3V) o -1 si hubo error.
     */
    int getBattery();

    /**
     * @brief Obtiene la versión del firmware del módulo "APP_VERSION".
     * @param outBuffer Buffer donde copiar el string (ej. "V1.0.4").
     * @param size Capacidad máxima del buffer.
     * @return true si tiene éxito.
     */
    bool getVersion(char* outBuffer, size_t size);

    /**
     * @brief Ejecuta un factory reset devolviendo al módulo a su estado de fábrica.
     * Cuidado: Esto borra todas las llaves LoRaWAN/Sigfox escritas.
     * @return true si el módulo responde afirmativamente.
     */
    bool factoryReset();

    /**
     * @brief Obtiene el tiempo local del módulo (ej. "2024-01-01 12:00:00").
     * @param outBuffer Buffer donde copiar el tiempo.
     * @param size Capacidad del buffer.
     * @return true si tiene éxito.
     */
    bool getLocalTime(char* outBuffer, size_t size);

    /**
     * @brief Obtiene el baudrate actual configurado en el módulo.
     * @return El baudrate (ej. 9600) o -1 si falla.
     */
    int getBaudrate();

    /**
     * @brief Configura el baudrate del módulo (por defecto suele ser 9600).
     * @param baudrate 9600, 19200, 38400, 57600, 115200.
     * @return true si tuvo éxito.
     */
    bool setBaudrate(int baudrate);

    /**
     * @brief Configura el nivel de verbosidad del módulo.
     * @param level El nivel de log deseado (0, 1, o 2).
     * @return true si tuvo éxito.
     */
    bool setVerboseLevel(int level);

private:
    UartDriver*       _driver;
    LSM1x0A_AtParser* _parser;
    bool              _initialized;
};

#endif // LSM1X0A_CONTROLLER_H
