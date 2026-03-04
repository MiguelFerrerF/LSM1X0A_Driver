\page lsm1x0a_sigfox_api_page Sigfox API Architecture

# LSM1x0A_Sigfox API Guide

This document describes the functions available in the `LSM1x0A_Sigfox` submodule, which handles Sigfox configuration and operation for the LSM1x0A controller. It serves as a reference for developers and agents wishing to maintain, extend, or integrate new features into the firmware.

## Visual Overview

### Operational Flow
@mermaid
graph LR
    Start((Start)) --> Config[Set RC Channel]
    Config --> Power[Set Radio Power]
    Power --> Payload[sendPayload / sendString]
    Payload --> DL{Downlink?}
    DL -->|Yes| Wait[Wait for +RX_H=...]
    Wait --> Extract[Extract RSSI / Time]
    DL -->|No| Ready[Ready for next Send]
    Extract --> Ready
@endmermaid

### Downlink & RSSI Extraction
@mermaid
sequenceDiagram
    participant User as App Code
    participant API as LSM1x0A_Sigfox
    participant Mod as Module (Sigfox)

    User->>API: sendPayload(data, true)
    API->>Mod: AT$SF=... ,1
    Mod-->>API: OK
    Note over API: Waiting for downlink...
    Mod-->>API: +RX_H= [RSSI, Timestamp, Data...]
    API-->>User: true (Transmission Finished)
    
    User->>API: getLastRxRSSI()
    API-->>User: -120 (dBm)
@endmermaid

## General Structure

The `LSM1x0A_Sigfox` class is designed to separate Sigfox-specific logic from the main controller, avoiding the God Object anti-pattern. All operations require an instance of `LSM1x0A_Controller` to interact with the hardware.

---

## 1. Keys and IDs Management
Allow for compiling and querying the identifiers necessary for Sigfox operations.
- **Getters:**
  - `getDeviceID`, `getInitialPAC`
  - Retrieve the current value from the module and populate the provided buffer securely.

---

## 2. Sigfox MAC Network Configuration
Allow adjusting network parameters and operation of the Sigfox stack.
- **Setters:**
  - `setRcChannel`, `setRadioPower`, `setPublicKeyMode`, `setPayloadEncryption`
  - Each function wraps the respective AT command and stores the variable securely in the local RAM cache.
- **Getters:**
  - `getRcChannel`, `getRadioPower`, `getPublicKeyMode`, `getPayloadEncryption`
  - They return the current value of the requested parameter.

---

## 3. Operations and Transmissions
Allow querying Sigfox network features.
- `sendBit`: Sends a single payload bit. Frame type 0. Includes optional `downlink` parameter to enable network response matching.
- `sendString`: Sends an ASCII string up to 12 bytes. Includes optional `downlink` parameter to enable network response matching.
- `sendPayload`: Sends a generic hexadecimal payload (byte array). Includes optional `downlink` parameter to enable network response matching.
- `sendOutOfBand`: Sends an Out-Of-Band (OOB) administrative message to the Sigfox Operator.
- `join`: Emulates a generic Join command by dispatching an empty `sendBit(true, true)` configuration sequence and tracking network state.

---

## 4. RX Data Extraction
Safely parses Sigfox Downlink requests directly from the AT Parser queue.
- `getLastRxRSSI`: Parses the `+RX_H=` response array and exposes the numerical representation of the Received Signal Strength.
- `getLastDownlinkTime`: Parses the `+RX_H=` sequence timestamp into standard POSIX structures (`struct tm*`) mapped directly to local memory instances.

---

## 5. Test and Certification Mode (RF Test)
Allow configuring and executing Sigfox radiofrequency tests.
- `testContinuousWave`, `testPRBS9`, `testMonarchScan`, `testMode`
- `testListenLoop`, `testSendLoop`, `testSendP2P`, `testReceiveP2P`
- Provides access to strictly-defined RF compliance methods enforced by the Sigfox certification standards.

---

## 6. Utilities and Maintenance
- `loadConfigFromModule`: Reads all current parameters utilizing physical communication methods. Stores parameters natively in local instance RAM cache properties instead of complex `struct` instances.
- `restoreConfig`: Triggers sequential recovery sequence applying all cached properties.
- `clearCache`: Demotes all cache entities to an uninitialized `-1` state.

---

## Usage Example
```cpp
LSM1x0A_Controller controller;
LSM1x0A_Sigfox sigfox(&controller);
sigfox.setRcChannel(LsmRCChannel::RC1);
sigfox.setRadioPower(14);
sigfox.join(); // Simulates downlink retrieval
int16_t rssi = sigfox.getLastRxRSSI();
```

---

## Extension and Maintenance
To add new features:
1. Define the method in the header (`LSM1x0A_Sigfox.h`).
2. Implement the logic in the corresponding file (`Setters`, `Getters`, or `Ops`).
3. Document the purpose, arguments, and possible return values.

This document must be kept updated with every relevant API change to ensure maintainability and facilitate future enhancements.
