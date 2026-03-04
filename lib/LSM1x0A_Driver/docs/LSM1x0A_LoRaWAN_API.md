\page lsm1x0a_lorawan_api_page LoRaWAN API Architecture

# LSM1x0A_LoRaWAN API Guide

This document describes the functions available in the `LSM1x0A_LoRaWAN` submodule, which handles LoRaWAN configuration and operation for the LSM1x0A controller. It serves as a reference for developers and agents wishing to maintain, extend, or integrate new features into the firmware.

## Visual Overview

### Operational State Machine (OTAA)
@mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> Configuring: Set Keys/Band
    Configuring --> Joining: join
    Joining --> Joining: Event JOIN_FAILED Retry
    Joining --> Connected: Event JOINED
    Connected --> Transmitting: sendData
    Transmitting --> Waiting: Request Downlink
    Waiting --> Connected: Event RX_DONE/Timeout
    Transmitting --> Connected: No Downlink
    Connected --> Idle: clearCache
@endmermaid

### Initialization & Join Flow
@mermaid
graph LR
    Start((Start)) --> Init[Instantiate LSM1x0A_LoRaWAN]
    Init --> Keys[Set DevEUI, AppEUI, AppKey]
    Keys --> Band[Set Band: EU868/US915/...]
    Band --> Join{join?}
    Join -->|OTAA| OTAA[Wait for +EVT:JOINED]
    Join -->|ABP| Connected
    OTAA --> Connected[Ready to Send]
    Connected --> Send[sendData]
@endmermaid

## General Structure

The `LSM1x0A_LoRaWAN` class is designed to separate LoRaWAN-specific logic from the main controller, avoiding the God Object anti-pattern. All operations require an instance of `LSM1x0A_Controller` to interact with the hardware.

---

## 1. Keys, IDs, and EUIs Management
Allow for compiling and querying the identifiers and keys necessary for authentication and LoRaWAN operation.
- **Setters:**
  - `setDevEUI`, `setAppEUI`, `setAppKey`, `setNwkKey`, `setDevAddr`, `setAppSKey`, `setNwkSKey`, `setNwkID`
  - They validate format and length, format logic into hex with delimiters, and send the corresponding AT command.
- **Getters:**
  - `getDevEUI`, `getAppEUI`, `getAppKey`, `getNwkKey`, `getDevAddr`, `getAppSKey`, `getNwkSKey`, `getNwkID`
  - Retrieve the current value from the module and return it as string or integer.

---

## 2. LoRaWAN MAC Network Configuration
Allow adjusting network parameters and operation of the LoRaWAN stack.
- **Setters:**
  - `setBand`, `setClass`, `setADR`, `setDataRate`, `setDutyCycle`, `setRx1Delay`, `setRx2Delay`, `setRx2DataRate`, `setRx2Frequency`, `setJoin1Delay`, `setJoin2Delay`, `setTxPower`, `setPingSlot`, `setNetworkType`, `setConfirmRetry`, `setUnconfirmRetry`, `setChannelMask`, `setDevNonce`, `resetDevNonce`
  - Each function validates the arguments and sends the proper AT command.
- **Getters:**
  - `getADR`, `getDataRate`, `getTxPower`, `getBand`, `getClass`, `getDutyCycle`, `getJoin1Delay`, `getJoin2Delay`, `getRx1Delay`, `getRx2Delay`, `getRx2DataRate`, `getRx2Frequency`, `getPingSlot`, `getConfirmRetry`, `getUnconfirmRetry`, `getNetworkType`, `getDevNonce`
  - They return the current value of the requested parameter.

---

## 3. Operational and Network Modes (LoRaWAN)
Allow controlling the join mode and data transmission.
- `setJoinMode`: Defines the join mode (OTAA/ABP).
- `join`: Starts the network join process, waits for success/failure events, and recovers the module upon error.
- `sendData`: Sends data over LoRaWAN, supports confirmed and unconfirmed messages, dynamically calculates timeouts, and recovers the module in case of failure.
- `requestLinkCheck`: Requests a link check; the response is received on the next uplink.

---

## 4. Test and Certification Mode (RF Test)
Allow configuring and executing radiofrequency tests and certification.
- `setRfTestConfig`, `startTxTest`, `startRxTest`, `startTxTone`, `startRxRssiTone`, `stopTest`, `setCertificationMode`, `startTxHoppingTest`, `startContinuousModulationTx`, `startContinuousModulationRx`, `sendCertificationPacket`
- Allow configuring frequencies, powers, modulation modes, and executing specific tests for certification.

---

## 5. P2P (Point-to-Point) Mode
Allow configuring and operating in point-to-point mode, outside the LoRaWAN stack.
- `setP2pConfig`, `sendP2pData`, `receiveP2pData`
- Allow setting direct communication parameters and sending/receiving data in P2P mode.

---

## 6. Utilities and Maintenance
- `loadConfigFromModule`: Reads all current parameters using the internal module getters and populates the RAM cache. Useful to instantiate via `controller->syncConfigToCache()` after boot up.
- `restoreConfig`: Restores session configuration to the module stored in its intermediate cache on the controller's RAM memory (only applies previously modified variables, ignoring uninitialized `UNKNOWN` ones).
- `clearCache`: Clears the internal cache (returning everything to `UNKNOWN`) to avoid inconsistencies or to force not overriding configurations upon a `restoreConfig`.
- `getLocalTime`: Retrieves the UTC time (seconds since Epoch) from the network via Downlink mapping.
- `getBaudrate` / `setBaudrate`: Gets or sets the module's target Baudrate, a property exclusively supported by LoRaWAN firmware variants.

---

## Design Considerations
- All functions validate arguments and return a boolean or integer depending on the operation's success.
- Setters format the data according to the LoRaWAN standard and the module's AT commands.
- Getters parse and clean the module's response to supply ready-to-use data.
- The modular design makes it easy to add new commands or modify existing ones.

---

## Usage Example
```cpp
LSM1x0A_Controller controller;
LSM1x0A_LoRaWAN lorawan(&controller);
lorawan.setDevEUI("0102030405060708");
lorawan.setAppKey("00112233445566778899AABBCCDDEEFF");
lorawan.setBand(LsmBand::EU868);
lorawan.join(LsmJoinMode::OTAA);
lorawan.sendData(1, "48656C6C6F", true);
```

---

## Extension and Maintenance
To add new features:
1. Define the method in the header (`LSM1x0A_LoRaWAN.h`).
2. Implement the logic in the corresponding file (`Setters`, `Getters`, or `Ops`).
3. Document the purpose, arguments, and possible return values.
4. Validate the impact on cache and event synchronization.

This document must be kept updated with every relevant API change to ensure maintainability and facilitate future enhancements.
