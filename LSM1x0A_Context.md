# LSM1x0A Driver Development Context

Este documento sirve como el contexto persistente principal para el desarrollo de un **driver de comunicación Serial robusto** para el módulo **LSM1x0A**. Este módulo integra un firmware dual-stack (LoRaWAN & Sigfox) y se controla íntegramente mediante comandos AT vía UART.

## 1. Especificaciones de Hardware e Interfaz
- **Interfaz:** UART (TX/RX)
- **Baudrate:** 
  - `9600` (Obligatorio para el modo Sigfox; baudrate por defecto de fábrica).
  - `115200` (Soportado opcionalmente para operar en modo LoRaWAN).
- **Formato de Comandos:** Todo envío debe ser en texto plano ASCII, comenzando con `AT` y terminando con la secuencia de retorno de carro (`\r` o `\r\n`).
- **Formato de Respuestas:** El módulo siempre responde utilizando la terminación estricta de nueva línea: `\r\n`.

## 2. Metas de la Arquitectura del Driver (Robustez)
Para lograr una comunicación a prueba de fallos, el desarrollo del driver debe cumplir con las siguientes directrices:

1. **Ejecución Síncrona Estricta:** El driver debe enviar un comando y bloquearse (con timeout) esperando el token finalizador `\r\nOK\r\n` o un token de error (`\r\nAT_ERROR\r\n`, etc.) antes de permitir el envío del siguiente comando.
2. **Vaciado de Buffer (Flushing):** Antes de enviar cualquier comando nuevo, el driver debe vaciar el buffer RX de la UART para descartar datos "basura" o ecos asíncronos anteriores que puedan desalinear el parser.
3. **Gestión de Timeouts Dinámicos:** 
   - *Comandos Rápidos* (ej: `AT+BAT`, `AT+DEUI=?`): Responden casi instantáneamente. Timeout corto (~500ms).
   - *Comandos de Red* (ej: `AT+JOIN`, `AT$SF`, `AT+SEND`): Responden después de involucrar ventanas de transmisión y recepción de radio. Pueden tardar de varios segundos a minutos. Timeout largo.
4. **Estrategia de Errores y Retries:** Manejar la respuesta `\r\nAT_BUSY_ERROR\r\n` implementando un *Backoff* (retraso antes del retry) en lugar de abortar o fallar inmediatamente.
5. **Eventos Asíncronos (URC):** El módulo genera eventos espontáneos (ej: `+EVT:JOINED`, `+EVT:RX_1...`). El driver debería contar con una tarea o rutina de interrupción de RX que atrape y enrute estos eventos hacia la aplicación superior, separándolos del flujo síncrono estándar.

## 3. Comportamientos Críticos y Sincronización
- **Wake-up Sequence (Modo Reposo):** Cuando el módulo lleva tiempo sin actividad o acaba de arrancar, el hardware UART puede estar en bajo consumo. El primer comando transmitido (`ATZ` o `AT`) suele ignorarse completamente porque el módulo lo usa para despertar su microcontrolador interno. El `LSM1x0A_AtParser` implementa el mecanismo de "Ping" consistente en enviar `AT\r\n` repetidamente hasta obtener un `OK`.
- **Cambio de Modo (`AT+MODE=0` o `AT+MODE=1`):** Según el análisis del firmware, cambiar el modo de red invoca la instrucción `NVIC_SystemReset();`. El driver detectará esto y **deberá esperar el tiempo de boot** del módulo antes de intentar ninguna otra comunicación.
- **Obtención de Variables (`AT+<CMD>=?`):** El firmware imprime el valor crudo en pantalla y luego finaliza con `\r\nOK\r\n`. El método `sendCommandWithResponse` del *AtParser* extrae lo que hay *entre* el eco del comando y el `\r\nOK\r\n`.

## 5. Arquitectura del Driver

El driver se divide en tres capas fundamentales para mantener la mantenibilidad estructural:

1. **Capa Física (`UartDriver`)**: 
   - **Responsabilidad:** Configurar periféricos ESP-IDF, manejar la interrupción RX asíncrona (tarea `uart_rx_task`), gestionar errores de hardware y agilizar lecturas crudas.
   - **Características:** Usa memoria en el stack `dtmp[1024]` durante la interrupción para prevenir leaks en destrucciones rudas de FreeRTOS.

2. **Capa de Enlace / Parser (`LSM1x0A_AtParser`):**
   - **Responsabilidad:** Procesar el tráfico entrante de UART por línea (`\n`), sincronizar comandos síncronos y reenviar eventos asíncronos (`+EVT:`) mediante un callback de usuario.
   - **Manejo Dinámico:** Contiene la lógica interna de `sendCommandWithResponse` y `parseRxMetadata`. Usa semáforos de FreeRTOS para aislar el hilo principal hasta la recepción del `\r\nOK\r\n` o `AT_ERROR`.

3. **Capa de Abstracción / Aplicación (`LSM1x0A_Controller`):**
   - **Responsabilidad:** Orquestar la creación y destrucción dinámica de `UartDriver` y `LSM1x0A_AtParser`.
   - **Paradigma Actual (Controlador Intuitivo):** Expone getters (`getBattery`, `getBaudrate`, `getVersion`) y setters nativos directos (Ej. `setDevEUI()`, `setClass()`) implementados mediantes llamadas directas a `sendCommand()` ocultando los búferes y formateos subyacentes.

4. **Definiciones (`LSM1x0A_Types.h`):**
   - Mantiene estrictamente enums compartidos (e.g., `AtError`, `LsmBand`, `LsmClass`) y diccionarios de comandos AT (`LsmAtCommand::...`) para evitar el uso de *magic strings* esparcidos en el código fuente.

## 6. Documentación de Referencia Interna
Para consultar los comandos, formatos y parámetros específicos durante el desarrollo, consultar:
1. `lib/LSM1x0A_Driver/docs/LSM1x0A_AT_Commands.md` -> Resumen de comandos disponibles y sintaxis general.
2. `lib/LSM1x0A_Driver/docs/LSM1x0A_LoRaWAN_Responses.md` -> Detalle exacto de la salida (`AT_PRINTF`) generada por el firmware en modo LoRaWAN.
3. `lib/LSM1x0A_Driver/docs/LSM1x0A_Sigfox_Responses.md` -> Detalle exacto de la salida (`AT_PRINTF`) generada por el firmware en modo Sigfox.
4. `lib/LSM1x0A_Driver/docs/LSM1x0A_LoRaWAN_API.md` -> Guía de arquitectura para los adaptadores de la API C++.

---

# 7. PlatformIO Project Context

Este apartado sirve como contexto para la compilación, carga y monitorización del proyecto. Incluye comandos específicos y detalles del entorno adaptados a esta máquina.

## Detalles del Entorno
- **Project Path:** `c:\Users\ORDENADOR 19\OneDrive - inBiot\Documentos\PlatformIO\Projects\LoRaWANController`
- **PlatformIO Executable:** `C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe`
- **Board:** `esp32dev` (ESP32 Dev Module genérico)
- **Port:** Autodetectado o manual (ej. `COM7`, `COM11`)
- **Monitor Speed:** `115200` (Definido en `platformio.ini`)
- **Arquitectura Base:**
    - **Framework:** Arduino con integración de FreeRTOS.
    - **Controladores:**
        - `UartDriver`: Capa física de interacción serial asíncrona dedicada. Maneja colas de FreeRTOS para aislar interrupciones RX.
        - `LSM1x0A_Controller`: API principal y orquestador de lógica del módulo dual-stack.
    - **Tests:** Framework Unity integrado con PlatformIO (`test_uartDriver`, etc.)
- **Librerías/Test:** `throwtheswitch/Unity` para las pruebas asíncronas de UART.

## Comandos

### 1. Build (Compilación)
Compila el código fuente del proyecto del firmware principal.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run
```

### 2. Clean (Limpieza)
Limpia los artefactos de compilación. Muy útil si hay errores extraños de dependencias o de cachés corruptas tras cambiar muchas configuraciones en `platformio.ini`.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run --target clean
```

### 3. Upload (Carga de Firmware)
Compila (si es necesario) y sube el firmware al ESP32 por USB o UART.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run --target upload
```

### 4. Test (Ejecución de Pruebas Unitarias)
Compila y sube el entorno de pruebas de un módulo específico (ej. el test de UART) validando en el hardware físico.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" test -e esp32dev -f LSM1x0A_Driver_Test/test_uartDriver
```

### 5. Monitor Serial
Abre el monitor interactivo de PlatformIO para ver los logs en vivo. Importante: Para ver el debug en crudo puede ser necesario hacer un script en Python externo debido al reset por DTR que aplica por defecto PlatformIO y que borra el bootlog primigenio del chip.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" device monitor
```
Para salir del monitor de PIO: Presiona `Ctrl+C`.

## Configuración (platformio.ini)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
```

## Progreso Reciente (Refactorización al Controlador Intuitivo)
Tras validar el hardware original, se decidió abandonar la complejidad extrema de *Shadow Configs* y Máquinas de Recuperación del repositorio original (actualmente respaldado en la carpeta `reference_controller`) en favor de un enfoque ultra robusto y limpio. 

1. **Memory Leaks Aislados:** Se implementó una prueba rigurosa de inicialización/destrucción dinámica en hardware repetida en `main.cpp`, asegurando 0 bytes de divergencia de Heap logrando un stack FreeRTOS hermético sin fugas.
2. **Setup Rápido:** Capacidad de instanciar un controller que por debajo autoinicializa la UART y el parser.
3. **Fase 1 (Terminada):** Implementación testada de comandos Getters (`getBattery`, `getBaudrate`, `getVersion`, `getLocalTime`, `getSigfoxVersion`, `getDeviceType`) y Setters Básicos (`setBaudrate`, `setVerboseLevel`, `setMode`, `startFwUpgrade`, `factoryReset`). Adición de un mecanismo resiliente de *Retry* y *Module Recovery* (Fallbacks de `ATZ` a GPIO Reset), incluyendo la posterior recuperación de configuraciones volátiles (`recoverModuleConfig`) y estado de conexión (`recoverModuleState`).
4. **Fase 2 (En progreso avanzado):** Implementación de la capa LoRaWAN nativa (ej: `setDevEUI()`, `setClass()`). Estas envuelven formateos en buferes transparentes listos para `sendCommand()`. 
   - *Nota:* Para evitar archivos kilométricos («God Objects»), la implementación de `LSM1x0A_LoRaWAN` se dividió en `LSM1x0A_LoRaWAN_Setters.cpp`, `LSM1x0A_LoRaWAN_Getters.cpp` y `LSM1x0A_LoRaWAN_Ops.cpp`.
   - *Sincronización Avanzada*: Se integró un bloque de FreeRTOS `EventGroup` dentro de `LSM1x0A_Controller` para interceptar comandos asíncronos (`+JOIN:`, `+TX:`, etc.) del parseador. Esto permite crear funciones como `lorawan.join(...)` y `lorawan.sendData(...)` aparentemente síncronas que se bloquean hasta recibir confirmación por red simulando un flujo secuencial sin bloquear el sistema operativo (RTOS `xEventGroupWaitBits`).
   - *Recuperación Caching Volátil*: Todo Setter sobreescribe un slot caché `_cached...` que luego es reaplicado masivamente si el módulo sufre un reset desatendido mediante `restoreConfig()`.
   - **Gestión Avanzada de Subbandas (Channel Mask):** Se refactorizaron las operaciones `setChannelMask` y `getChannelMask` para abstraer la complejidad de regiones híbridas (US915 / AU915 / CN470). El driver ahora maneja asincronía en las solicitudes AT, traduce nativamente constantes bit a bit (`LsmSubBand`) en cadenas AT multi-bloque hexagonales correctamente formateadas (ej. los 6 bloques US915) independientemente de la región activa.

---

# 8. Documentación de Referencia Interna
Las guías de referencia y comandos AT ya no se encuentran dispersas en la raíz del proyecto. **Han sido unificadas dentro del submódulo del driver** bajo la carpeta `lib/LSM1x0A_Driver/docs/` para no contaminar el root directory y relacionarlas estricamente con los encabezados `.h`.

---

# 7. Reglas de Desarrollo Autónomo (AI Rules)

Para mantener la calidad y fiabilidad del firmware, **todo el código autogenerado** o modificado por asistentes de IA en este proyecto debe cumplir estrictamente las siguientes directivas expuestas en los archivos base `.cursorrules` / `.windsurfrules`:

1. **Compilación y Testeo Obligatorios:** 
   Cada vez que se añada una nueva funcionalidad, se modifique el comportamiento del controlador o se modifique un archivo fuente (`.cpp` / `.h`), el Agente DEBE compilar el código (`pio run -e esp32dev`). Si el agente considera que el cambio es a nivel de despliegue, también DEBE subirlo al dispositivo y chequear el monitor serial usando los comandos de PlatformIO detallados en la sección 6. ¡No se entregará código sin verificar que compila exitosamente en el entorno físico/target!

2. **Monitorización Autónoma Obligatoria:**
   Si el agente lanza un cambio que debe ser testeado lógicamente, es completamente responsable de correr el monitor serial y analizar los logs impresos por el ESP32. Debe parsear y diagnosticar las salidas él mismo usando sus tools internas. 

3. **Código Profesional, Modular y Depurado:**
   Se exige un estándar de "Clean Code". Todo código nuevo debe:
   - Estar separado en funciones simples y atómicas que hagan una sola cosa (Single Responsibility Principle).
   - Mantener una estructura organizada (separación de la definición en `.h` y la implementación en `.cpp`).
   - Evitar "God Objects" o rutinas kilométricas. Si una función crece demasiado, debe subdividirse en helpers privados auxiliares.
   - Aplicar separación por casos de uso. Por ejemplo, separar la lógica de parseo, capa física y capa de abstracción.
