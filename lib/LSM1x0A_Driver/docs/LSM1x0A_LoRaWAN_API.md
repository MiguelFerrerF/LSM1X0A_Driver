# Guía de la API LSM1x0A_LoRaWAN

Este documento describe las funciones disponibles en el submódulo `LSM1x0A_LoRaWAN`, encargado de la configuración y operación LoRaWAN para el controlador LSM1x0A. Sirve como referencia para desarrolladores y agentes que deseen mantener, extender o integrar nuevas funcionalidades en el firmware.

## Estructura General

La clase `LSM1x0A_LoRaWAN` está diseñada para separar la lógica específica de LoRaWAN del controlador principal, evitando el antipatrón God Object. Todas las operaciones requieren una instancia de `LSM1x0A_Controller` para interactuar con el hardware.

---

## 1. Gestión de Llaves, IDs y EUIs
Permiten configurar y consultar los identificadores y llaves necesarias para la autenticación y operación LoRaWAN.
- **Setters:**
  - `setDevEUI`, `setAppEUI`, `setAppKey`, `setNwkKey`, `setDevAddr`, `setAppSKey`, `setNwkSKey`, `setNwkID`
  - Validan formato y longitud, formatean a hexadecimal con delimitadores y envían el comando AT correspondiente.
- **Getters:**
  - `getDevEUI`, `getAppEUI`, `getAppKey`, `getNwkKey`, `getDevAddr`, `getAppSKey`, `getNwkSKey`, `getNwkID`
  - Recuperan el valor actual desde el módulo y lo devuelven como string o entero.

---

## 2. Configuración de Red Mac (LoRaWAN)
Permiten ajustar parámetros de red y operación del stack LoRaWAN.
- **Setters:**
  - `setBand`, `setClass`, `setADR`, `setDataRate`, `setDutyCycle`, `setRx1Delay`, `setRx2Delay`, `setRx2DataRate`, `setRx2Frequency`, `setJoin1Delay`, `setJoin2Delay`, `setTxPower`, `setPingSlot`, `setNetworkType`, `setConfirmRetry`, `setUnconfirmRetry`, `setChannelMask`, `setDevNonce`, `resetDevNonce`
  - Cada función valida los argumentos y envía el comando AT adecuado.
- **Getters:**
  - `getADR`, `getDataRate`, `getTxPower`, `getBand`, `getClass`, `getDutyCycle`, `getJoin1Delay`, `getJoin2Delay`, `getRx1Delay`, `getRx2Delay`, `getRx2DataRate`, `getRx2Frequency`, `getPingSlot`, `getConfirmRetry`, `getUnconfirmRetry`, `getNetworkType`, `getDevNonce`
  - Devuelven el valor actual del parámetro solicitado.

---

## 3. Modos Operacionales y Red (LoRaWAN)
Permiten controlar el modo de unión y la transmisión de datos.
- `setJoinMode`: Define el modo de unión (OTAA/ABP).
- `join`: Inicia el proceso de unión a la red, espera eventos de éxito o fallo y recupera el módulo si hay error.
- `sendData`: Envía datos por LoRaWAN, soporta mensajes confirmados y no confirmados, calcula timeouts dinámicamente y recupera el módulo en caso de fallo.
- `requestLinkCheck`: Solicita un chequeo de enlace, la respuesta se recibe en el siguiente uplink.

---

## 4. Modo de Pruebas y Certificación (RF Test)
Permiten configurar y ejecutar pruebas de radiofrecuencia y certificación.
- `setRfTestConfig`, `startTxTest`, `startRxTest`, `startTxTone`, `startRxRssiTone`, `stopTest`, `setCertificationMode`, `startTxHoppingTest`, `startContinuousModulationTx`, `startContinuousModulationRx`, `sendCertificationPacket`
- Permiten configurar frecuencias, potencias, modos de modulación y ejecutar pruebas específicas para certificación.

---

## 5. Modo P2P (Punto a Punto)
Permiten configurar y operar en modo punto a punto, fuera del stack LoRaWAN.
- `setP2pConfig`, `sendP2pData`, `receiveP2pData`
- Permiten establecer parámetros de comunicación directa y enviar/recibir datos en modo P2P.

---

## 6. Utilidades y Mantenimiento
- `loadConfigFromModule`: Lee todos los parámetros actuales usando los getters internos al módulo y puebla la RAM caché. Útil instanciarlo a través de `controller->syncConfigToCache()` después de iniciar.
- `restoreConfig`: Restaura al módulo la configuración de sesión almacenada en su caché intermedio en la memoria RAM del controlador (solo aplica las variables previamente modificadas, saltándose las no inicializadas `UNKNOWN`).
- `clearCache`: Limpia la caché interna (volviendo todo a `UNKNOWN`) para evitar inconsistencias u obligar a no sobreescribir configuraciones en un `restoreConfig`.

---

## Consideraciones de Diseño
- Todas las funciones validan argumentos y devuelven un valor booleano o entero según el éxito de la operación.
- Los setters formatean los datos según el estándar LoRaWAN y los comandos AT del módulo.
- Los getters extraen y limpian la respuesta del módulo para entregar datos listos para usar.
- El diseño modular permite añadir nuevos comandos o modificar los existentes de forma sencilla.

---

## Ejemplo de Uso
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

## Extensión y Mantenimiento
Para añadir nuevas funciones:
1. Definir el método en el header (`LSM1x0A_LoRaWAN.h`).
2. Implementar la lógica en el archivo correspondiente (`Setters`, `Getters` u `Ops`).
3. Documentar el propósito, argumentos y posibles valores de retorno.
4. Validar el impacto en la caché y la sincronización de eventos.

Este documento debe mantenerse actualizado con cada cambio relevante en la API para asegurar la mantenibilidad y facilitar futuras mejoras.
