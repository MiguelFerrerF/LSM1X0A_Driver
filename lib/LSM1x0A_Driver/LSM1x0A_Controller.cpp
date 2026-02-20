#include "LSM1x0A_Controller.h"

LSM1x0A_Controller::LSM1x0A_Controller()
{
    _driver = new UartDriver();
    _parser = new LSM1x0A_AtParser();
    _initialized = false;
}

LSM1x0A_Controller::~LSM1x0A_Controller()
{
    end();
    if (_parser) {
        delete _parser;
        _parser = nullptr;
    }
    if (_driver) {
        delete _driver;
        _driver = nullptr;
    }
}

bool LSM1x0A_Controller::begin(AtEventCallback callback, void* ctx)
{
    if (_initialized) return true;

    if (!_driver || !_parser) {
        return false;
    }

    // Inicializamos el parser pasándole el UartDriver que construimos.
    // El parser internamente llama a _driver->init()
    if (!_parser->init(_driver, callback, ctx)) {
        return false;
    }

    _initialized = true;
    return true;
}

void LSM1x0A_Controller::end()
{
    if (!_initialized) return;

    if (_driver) {
        _driver->deinit();
    }
    _initialized = false;
}

bool LSM1x0A_Controller::wakeUp()
{
    if (!_initialized || !_parser) return false;
    return _parser->wakeUp();
}

AtError LSM1x0A_Controller::sendCommand(const char* cmd, uint32_t timeoutMs)
{
    if (!_initialized || !_parser) return AtError::GENERIC_ERROR;
    return _parser->sendCommand(cmd, timeoutMs);
}

AtError LSM1x0A_Controller::sendCommandWithResponse(
    const char* cmd, 
    char* outBuffer, 
    size_t outSize, 
    const char* expectedTag, 
    uint32_t timeoutMs)
{
    if (!_initialized || !_parser) return AtError::GENERIC_ERROR;

    if (!outBuffer || outSize == 0) return AtError::PARAM_ERROR;
    outBuffer[0] = '\0';

    return _parser->sendCommandWithResponse(cmd, expectedTag, outBuffer, outSize, timeoutMs);
}

// =========================================================================
// COMANDOS AT BÁSICOS / GENERALES
// =========================================================================

int LSM1x0A_Controller::getBattery()
{
    char buf[16];
    if (sendCommandWithResponse(LsmAtCommand::BATTERY, buf, sizeof(buf), nullptr, 1000) != AtError::OK) {
        return -1;
    }
    // Convertir de char "3300" a int
    return atoi(buf);
}

bool LSM1x0A_Controller::getVersion(char* outBuffer, size_t size)
{
    if (!outBuffer || size == 0) return false;
    
    // Comando AT+VER=?
    // Puede venir con "APP_VERSION:" o nada, este módulo devuelve múltiples líneas
    // usaremos el base parser que agarra la respuesta principal.
    AtError err = sendCommandWithResponse(LsmAtCommand::FW_VERSION, outBuffer, size, "APP_VERSION:", 2000);
    
    // Si la etiqueta APP_VERSION no se encontraba, capturamos todo
    if (err != AtError::OK) {
       err = sendCommandWithResponse(LsmAtCommand::FW_VERSION, outBuffer, size, nullptr, 2000);
    }
    return err == AtError::OK;
}

bool LSM1x0A_Controller::factoryReset()
{
    return sendCommand(LsmAtCommand::FACTORY_RESET, 5000) == AtError::OK;
}

bool LSM1x0A_Controller::getLocalTime(char* outBuffer, size_t size)
{
    if (!outBuffer || size == 0) return false;
    return sendCommandWithResponse(LsmAtCommand::LOCAL_TIME, outBuffer, size, nullptr, 1000) == AtError::OK;
}

int LSM1x0A_Controller::getBaudrate()
{
    char buf[16];
    // Comando pidiendo el baudrate: AT+BAUDRATE=?
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "%s?", LsmAtCommand::BAUDRATE);

    if (sendCommandWithResponse(cmd, buf, sizeof(buf), nullptr, 1000) != AtError::OK) {
        return -1;
    }
    return atoi(buf);
}

bool LSM1x0A_Controller::setBaudrate(int baudrate)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::BAUDRATE, baudrate);
    return sendCommand(cmd, 1000) == AtError::OK;
}

bool LSM1x0A_Controller::setVerboseLevel(int level)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "%s%d", LsmAtCommand::VERBOSE_LEVEL, level);
    return sendCommand(cmd, 1000) == AtError::OK;
}
