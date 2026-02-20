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

## 3. Códigos de Respuesta Esperados (El Parser)
La máquina de estados del driver debe buscar explícitamente estas cadenas exactas como terminadores.

### Respuestas Universales
- `\r\nOK\r\n` -> Éxito.
- `\r\nAT_ERROR\r\n` -> Error general / comando inválido.
- `\r\nAT_PARAM_ERROR\r\n` -> Parámetro inválido o fuera de rango.
- `\r\nAT_BUSY_ERROR\r\n` -> Módulo ocupado.
- `\r\nAT_TEST_PARAM_OVERFLOW\r\n` -> Overflow de parámetros.
- `\r\nAT_RX_ERROR\r\n` -> Error en la UART interna del módulo.

### Respuestas Específicas: LoRaWAN
- `\r\nAT_NO_NETWORK_JOINED\r\n` -> Intento de enviar Payload sin estar unido a la red.
- `\r\nAT_DUTYCYCLE_RESTRICTED\r\n` -> Límite legal de ETSI Duty Cycle alcanzado.
- `\r\nAT_CRYPTO_ERROR\r\n` -> Error de encriptación Mac.

### Respuestas Específicas: Sigfox
- `\r\nAT_TX_TIMEOUT\r\n` -> Timeout enviando a la red (Normalmente LBT/Límites de canal).
- `\r\nAT_RX_TIMEOUT\r\n` -> Fin del tiempo de espera para el downlink de Sigfox.
- `\r\nAT_LIB_ERROR\r\n` -> Error en el stack propietario de Sigfox.

## 4. Comportamientos Críticos de Comandos
- **Wake-up Sequence (Modo Reposo):** Cuando el módulo lleva tiempo sin actividad o acaba de arrancar, el hardware UART puede estar en bajo consumo. El primer comando transmitido (`ATZ` o `AT`) suele ignorarse completamente (no devuelve ni eco ni `ERROR`) porque el módulo lo usa para despertar su microcontrolador interno. El driver de comunicaciones (`AtParser` o capa superior) debe implementar un mecanismo de "Ping" consistente en enviar `AT\r\n` repetidamente (varias veces con retardos de ~500ms) hasta obtener un `OK` antes de asumir que el módulo está listo para recibir comandos complejos.
- **Cambio de Modo (`AT+MODE=0` o `AT+MODE=1`):** Según el análisis del firmware, cambiar el modo de red invoca la instrucción `NVIC_SystemReset();`. El driver detectará esto y **deberá esperar el tiempo de boot** del módulo antes de intentar ninguna otra comunicación.
- **Obtención de Variables (`AT+<CMD>=?`):** Cuando el driver interroga por un valor, el firmware generalmente imprime el valor crudo en pantalla (con o sin comillas, según el caso) y luego finaliza el bloque con `\r\nOK\r\n`. El parser debe ser capaz de extraer lo que hay *entre* el eco del comando y el `\r\nOK\r\n`.

## 5. Arquitectura del Driver Propuesta

El driver debe separarse funcionalmente en capas para mantener el código testeable y modular, especialmente en FreeRTOS:

1. **Capa Física (`UartDriver`)**: 
   - **Responsabilidad:** Configurar periféricos ESP-IDF, manejar la interrupción RX asíncrona mediante una tarea dedicada (existente: `uart_rx_task`), gestionar errores de hardware (overflow) y notificar hacia arriba bloque a bloque mediante un callback.
   - **Mejora:** Función pública `flush()` para garantizar el requisito de vaciado de buffer antes de enviar comandos sincrónicos.
2. **Capa de Comandos (Futura iteración)**:
   - **Responsabilidad:** Recibir los fragmentos del callback físico y ensamblar líneas enteras, bloqueando la tarea llamante de alto nivel hasta que ocurra un *Timeout* o se extraiga la respuesta. *Actualmente pospuesto, el parseo se hará manualmente o en iteraciones futuras.*
3. **Capa de Aplicación (API Segura) (Futura iteración)**:
   - **Responsabilidad:** Exponer llamadas de alto nivel como `getBattery()`.

## 6. Documentación de Referencia Interna
Para consultar los comandos, formatos y parámetros específicos durante el desarrollo, consultar:
1. `LSM1x0A_AT_Commands.md` -> Resumen de comandos disponibles y sintaxis general.
2. `LSM1x0A_LoRaWAN_Responses.md` -> Detalle exacto de la salida (`AT_PRINTF`) generada por el firmware en modo LoRaWAN.
3. `LSM1x0A_Sigfox_Responses.md` -> Detalle exacto de la salida (`AT_PRINTF`) generada por el firmware en modo Sigfox.
