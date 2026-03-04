@page lsm1x0a_context_page Architecture & Context

# LSM1x0A Driver Development Context

This document serves as the primary persistent context for the development of a **robust Serial communication driver** for the **LSM1x0A** module. This module integrates a dual-stack firmware (LoRaWAN & Sigfox) and is fully controlled via AT commands over UART.
## 1. Hardware Specifications and Interface
 - **Interface:** UART (TX/RX)
 - **Baudrate:** 
   - `9600` (Mandatory for Sigfox mode; factory default baudrate).
   - `115200` (Optionally supported for operating in LoRaWAN mode).
 - **Command Format:** All transmissions must be in plain ASCII text, starting with `AT` and ending with the carriage return sequence (`\r` or `\r\n`).
 - **Response Format:** The module always responds using a strict newline termination: `\r\n`.
## 2. Driver Architecture Goals (Robustness)
To achieve fail-safe communication, driver development must adhere to the following guidelines:
 1. **Strict Synchronous Execution:** The driver must send a command and block (with a timeout) waiting for the finalizer token `\r\nOK\r\n` or an error token (`\r\nAT_ERROR\r\n`, etc.) before allowing the transmission of the next command.
 2. **Buffer Flushing:** Before sending any new command, the driver must flush the UART RX buffer to discard "garbage" data or previous asynchronous echoes that could misalign the parser.
 3. **Dynamic Timeout Management:** 
    - *Fast Commands* (e.g., `AT+BAT`, `AT+DEUI=?`): Respond almost instantaneously. Short timeout (~500ms).
    - *Network Commands* (e.g., `AT+JOIN`, `AT$SH`/`AT$SF`, `AT+SEND`): Respond after involving radio transmission and reception windows. They can take anywhere from a few seconds to minutes. Long timeout.
 4. **Error Strategy and Retries:** Handle the `\r\nAT_BUSY_ERROR\r\n` response by implementing a *Backoff* (delay before retry) instead of aborting or failing immediately.
 5. **Asynchronous Events (URC):** The module generates spontaneous events (e.g., `+EVT:JOINED`, `+EVT:RX_1...`). The driver should have an RX interrupt routine or task that catches and routes these events to the upper application layer, separating them from the standard synchronous flow.
### Sequence diagram of AT Execution Model
@mermaid
sequenceDiagram
   participant App as Application Layer
   participant AT as AtParser + UartDriver
   participant Mod as LSM1x0A Module
   %% Synchronous Execution
   Note over App, Mod: Synchronous Command (e.g. Get DevEUI)
   App->>AT: getDevEUI()
   AT->>Mod: "AT+DEUI=?\r\n"
   Mod-->>AT: "1122334455667788\r\n"
   Mod-->>AT: "OK\r\n"
   AT-->>App: "1122334455667788" (Unblocks)
   %% Asynchronous Execution
   Note over App, Mod: Asynchronous Process (e.g. Join Network)
   App->>AT: join()
   AT->>Mod: "AT+JOIN=1\r\n"
   Mod-->>AT: "OK\r\n"
   Note over AT: Wait dynamically (EventGroup)
   Mod-->>AT: "+EVT:JOINED\r\n"
   AT-->>App: true (Unblocks)
   %% Retries
   Note over App, Mod: Error & Backoff Handle
   App->>AT: setMode()
   AT->>Mod: "AT+MODE=1\r\n"
   Mod-->>AT: "AT_BUSY_ERROR\r\n"
   Note over AT: Internal Backoff (500ms)
   AT->>Mod: "AT+MODE=1\r\n"
   Mod-->>AT: "OK\r\n"
   AT-->>App: true
@endmermaid

## 3. Critical Behaviors and Synchronization
- **Wake-up Sequence (Sleep Mode):** When the module has been inactive for some time or has just booted, the UART hardware might be in low power mode. The first transmitted command (`ATZ` or `AT`) is often completely ignored because the module uses it to wake up its internal microcontroller. The `LSM1x0A_AtParser` implements a "Ping" mechanism that consists of sending `AT\r\n` repeatedly until it gets an `OK`.
- **Mode Switching (`AT+MODE=0` or `AT+MODE=1`):** According to the firmware analysis, changing the network mode invokes the `NVIC_SystemReset();` instruction. The driver will detect this and **must wait the module's boot time** before attempting any other communication.
- **Retrieving Variables (`AT+<CMD>=?`):** The firmware prints the raw value on screen and then finishes with `\r\nOK\r\n`. The `sendCommandWithResponse` method from the *AtParser* extracts what is *between* the command echo and the `\r\nOK\r\n`.

## 5. Driver Architecture

The driver is divided into three foundational layers to maintain structural maintainability:

1. **Physical Layer (`UartDriver`)**: 
- **Responsibility:** Configure ESP-IDF peripherals, handle the asynchronous RX interrupt (`uart_rx_task`), manage hardware errors, and expedite raw readings.
- **Features:** Uses a stack memory buffer `dtmp[1024]` during the interrupt to prevent leaks on harsh FreeRTOS destruction.

2. **Link / Parser Layer (`LSM1x0A_AtParser`):**
- **Responsibility:** Process incoming UART traffic line by line (`\n`), synchronize synchronous commands, and forward asynchronous events (`+EVT:`) internally to the Controller.
- **Dynamic Handling:** Contains the internal logic for `sendCommandWithResponse` and `parseRxMetadata`. Uses FreeRTOS semaphores to isolate the main thread until the reception of `\r\nOK\r\n` or `AT_ERROR`.

3. **Abstraction / Application Layer (`LSM1x0A_Controller`):**
- **Responsibility:** Orchestrate the dynamic creation and destruction of `UartDriver` and `LSM1x0A_AtParser`.
- **Current Paradigm (Intuitive Controller):** Exposes getters (`getBattery`, `getBaudrate`, `getVersion`) and direct native setters (e.g., `setDevEUI()`, `setClass()`) implemented through direct calls to `sendCommand()` hiding the underlying buffers and formatting.

4. **Definitions (`LSM1x0A_Types.h`):**
- Strictly maintains shared enums (e.g., `AtError`, `LsmBand`, `LsmClass`) and AT command dictionaries (`LsmAtCommand::...`) to avoid the use of scattered *magic strings* in the source code.

### Visual Architecture Diagram
@mermaid
graph TD
   %% Estilos
   classDef physical fill:#e2e8f0,stroke:#64748b,stroke-width:2px,color:#0f172a;
   classDef parser fill:#bae6fd,stroke:#0284c7,stroke-width:2px,color:#0f172a;
   classDef api fill:#bbf7d0,stroke:#16a34a,stroke-width:2px,color:#0f172a;
   classDef userApp fill:#fef08a,stroke:#ca8a04,stroke-width:2px,color:#0f172a;
   classDef external fill:#fecaca,stroke:#dc2626,stroke-width:2px,color:#0f172a;

   %% Nodos
   User["User Application / main.cpp"]:::userApp

   subgraph Driver ["LSM1x0A Driver"]
      direction TB
      API["LSM1x0A_Controller<br/>High level get/set"]:::api
      Parser["LSM1x0A_AtParser<br/>Logic & Synchronization"]:::parser
      Physical["UartDriver<br/>RTOS Queues & ISR"]:::physical
      Logger["LSM1x0A_Logger<br/>Event Global Tracker"]:::api
   end

   Hardware[("LSM1x0A Module<br/>STM32WL")]:::external

   %% Conexiones (Corregidas flechas dobles <-->)
   User <--> API
   User -- "Subscribes & Listens" --> Logger
   API -- "Logs native events" --> Logger
   API -- "Synchronous Tx Commands" --> Parser
   Parser -- "Asynchronous +EVT" --> API
   Parser -- "Raw Tx Payload" --> Physical
   Physical -- "Parsed Line (\n)" --> Parser
   Physical <--> Hardware
@endmermaid

## 6. Internal Reference Documentation
To consult specific commands, formats, and parameters during development, see:
1. `lib/LSM1x0A_Driver/docs/LSM1x0A_AT_Commands.md` -> Summary of available commands and general syntax.
2. `lib/LSM1x0A_Driver/docs/LSM1x0A_LoRaWAN_Responses.md` -> Exact details of the output (`AT_PRINTF`) generated by the firmware in LoRaWAN mode.
3. `lib/LSM1x0A_Driver/docs/LSM1x0A_Sigfox_Responses.md` -> Exact details of the output (`AT_PRINTF`) generated by the firmware in Sigfox mode.
4. `lib/LSM1x0A_Driver/docs/LSM1x0A_LoRaWAN_API.md` -> Architecture guide for the C++ API adapters.

---

# 7. PlatformIO Project Context
This section serves as context for compiling, flashing, and monitoring the project. It includes specific commands and environment details adapted to this machine.

## Environment Details
- **Project Path:** `c:\Users\ORDENADOR 19\OneDrive - inBiot\Documentos\PlatformIO\Projects\LoRaWANController`
- **PlatformIO Executable:** `C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe`
- **Board:** `esp32dev` (Generic ESP32 Dev Module)
- **Port:** Auto-detected or manual (e.g., `COM7`, `COM11`)
- **Monitor Speed:** `115200` (Defined in `platformio.ini`)
- **Base Architecture:**
- **Framework:** ESP-IDF / FreeRTOS (Arduino dependencies removed from the core driver for portability).
- **Controllers:**
- `UartDriver`: Dedicated asynchronous serial interaction physical layer. Manages FreeRTOS queues to isolate RX interrupts.
- `LSM1x0A_Controller`: Main API and logic orchestrator of the dual-stack module.
- **Logging:** Native logger (`LSM1x0A_Logger`) with adjustable verbosity levels at compile-time and runtime.
- **Tests:** Unity framework integrated with PlatformIO (`test_uartDriver`, etc.)
- **Libraries/Test:** `throwtheswitch/Unity` for asynchronous UART tests.

## Commands
### 1. Build
Compiles the project's main firmware source code.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run
```

### 2. Clean
Cleans build artifacts. Highly useful if there are strange dependency or corrupted cache errors after changing many settings in `platformio.ini`.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run --target clean
```

### 3. Upload (Firmware Flash)
Compiles (if necessary) and uploads the firmware to the ESP32 via USB or UART.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" run --target upload
```

### 4. Test (Unit Testing Execution)
Compiles and uploads the test environment of a specific module (e.g., the UART test) validating on the physical hardware.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" test -e esp32dev -f LSM1x0A_Driver_Test/test_uartDriver
```

### 5. Serial Monitor
Opens the interactive PlatformIO monitor to see live logs. Important: To see raw debug output, it may be necessary to write an external Python script due to the baseline DTR reset applied by PlatformIO that wipes the chip's initial bootlog.
```powershell
& "C:\Users\ORDENADOR 19\.platformio\penv\Scripts\platformio.exe" device monitor
```
To exit the PIO monitor: Press `Ctrl+C`.

## Configuration (platformio.ini)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
```
