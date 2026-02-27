# LSM1x0A AT Commands Reference

Este documento recopila todos los comandos AT disponibles para el módulo LSM1x0A, tanto para su firmware LoRaWAN como Sigfox. El módulo recibe estos comandos a través de su interfaz serial (TX/RX).

---

## Interfaz Serial (UART)
- **Baudrate Rango**: 9600 (por defecto) o 115200.
  - *Nota:* Para funcionar en modo Sigfox, el baudrate **debe** estar configurado a 9600.
- **Formato del comando**: Todos los comandos comienzan con `AT`. Algunos usan el formato `AT+<CMD>`, `AT$<CMD>` o `AT<CMD>`.
- **Sintaxis común**:
  - `AT+<CMD>?` : Ayuda sobre el comando.
  - `AT+<CMD>` : Ejecutar el comando.
  - `AT+<CMD>=<valor>` : Establecer el valor.
  - `AT+<CMD>=?` : Obtener el valor actual.

---

## Códigos de Respuesta Universales
El parser de comandos AT devuelve las siguientes respuestas comunes. Para ver el detalle exacto de las respuestas por puerto serial (incluyendo variables impresas y datos crudos) que arroja *cada comando específico*, consulta los siguientes documentos detallados:
- [Respuestas detalladas LoRaWAN](file:///c:/Users/ORDENADOR%2019/OneDrive%20-%20inBiot/Documentos/PlatformIO/Projects/LoRaWANController/LSM1x0A_LoRaWAN_Responses.md)
- [Respuestas detalladas Sigfox](file:///c:/Users/ORDENADOR%2019/OneDrive%20-%20inBiot/Documentos/PlatformIO/Projects/LoRaWANController/LSM1x0A_Sigfox_Responses.md)
| `\r\nAT_PARAM_ERROR\r\n` | Parámetros incorrectos, fuera de rango o mal formateados. |
| `\r\nAT_BUSY_ERROR\r\n` | El módulo está ocupado ejecutando otra operación. |
| `\r\nAT_TEST_PARAM_OVERFLOW\r\n` | Desbordamiento de los parámetros o buffer interno. |
| `\r\nAT_RX_ERROR\r\n` | Error durante la recepción serial. |
| `\r\nerror unknown\r\n` | Código de error desconocido o máximo alcanzado. |

### Respuestas Específicas LoRaWAN
| Respuesta | Descripción |
| :--- | :--- |
| `\r\nAT_NO_NETWORK_JOINED\r\n` | Intento de enviar datos sin haberse unido a la red (JOIN). |
| `\r\nAT_NO_CLASS_B_ENABLE\r\n` | Intento de operar en Clase B sin estar habilitada. |
| `\r\nAT_DUTYCYCLE_RESTRICTED\r\n` | Restricción por sobrepasar el ETSI Duty Cycle del canal/banda. |
| `\r\nAT_CRYPTO_ERROR\r\n` | Error general en la capa criptográfica MAC de LoRaWAN. |

### Respuestas Específicas Sigfox
| Respuesta | Descripción |
| :--- | :--- |
| `\r\nAT_LIB_ERROR\r\n` | Error interno en la librería de Sigfox. |
| `\r\nAT_TX_TIMEOUT\r\n` | Tiempo de espera de transmisión agotado (Canal ocupado/CS). |
| `\r\nAT_RX_TIMEOUT\r\n` | Tiempo de espera de recepción agotado. |
| `\r\nAT_RECONF_ERROR\r\n` | Error durante la reconfiguración. |

---

## Comandos AT Comunes (Sistema / Interfaz)

| Comando | Ayuda | Descripción / Uso |
| :--- | :--- | :--- |
| `ATZ` | `ATZ` | Reinicia/Resetea el procesador (Trigger MCU reset). |
| `AT+RFS` / `AT$RFS` | `AT+RFS` o `AT$RFS` | Restaura los ajustes de fábrica en la memoria EEPROM. |
| `AT+VL` | `AT+VL=<Level>` | Nivel de Verbose/Debug. `[0:Off .. 3:High]`. |
| `AT+LTIME` | `AT+LTIME` | Obtiene la hora local en formato UTC (Solo LoRaWAN). |
| `AT+BAUDRATE` | `AT+BAUDRATE` | Establece el baudrate de la UART. Valores permitidos: 9600, 115200. Requiere reiniciar (Solo LoRaWAN / general). |
| `AT+BAT` | `AT+BAT` | Devuelve el nivel de la batería en milivoltios (mV). |
| `AT+MODE` | `AT+MODE` | Selecciona la tecnología de radio. `0:Sigfox`, `1:LoRa`. (Requiere baudrate 9600 para Sigfox). |
| `AT+FW` | `AT+FW` | Activa el modo actualización de firmware (Ota/Serial FW upgrade mode). |
| `AT+VER` | `AT+VER` | Devuelve la versión de firmware (FW version). |
| `AT$SSWVER` | `AT$SSWVER` | Devuelve la versión de software del Stack (SW Version). |

---

## Comandos AT: LoRaWAN

### 1. Gestión de Llaves, IDs y EUIs
| Comando | Formato Set | Descripción |
| :--- | :--- | :--- |
| `AT+APPEUI` | `AT+APPEUI=<XX:XX:XX:XX:XX:XX:XX:XX>` | (O `+JOINEUI`) Obtiene o establece el Application EUI (Join EUI). |
| `AT+DEUI` | `AT+DEUI=<XX:XX:XX:XX:XX:XX:XX:XX>` | Obtiene o establece el Device EUI. |
| `AT+DADDR` | `AT+DADDR=<XXXXXXXX>` | Obtiene o establece el Device Address. |
| `AT+APPKEY` | `AT+APPKEY=<16 Bytes (Hex)>` | Obtiene o establece la Application Key. |
| `AT+NWKKEY` | `AT+NWKKEY=<16 Bytes (Hex)>` | Obtiene o establece la Network Key. |
| `AT+APPSKEY` | `AT+APPSKEY=<16 Bytes (Hex)>` | Obtiene o establece la Application Session Key. |
| `AT+NWKSKEY` | `AT+NWKSKEY=<16 Bytes (Hex)>` | Obtiene o establece la Network Session Key. |
| `AT+NWKID` | `AT+NWKID=<NwkID>` | Obtiene o establece el Network ID `[0..127]`. |

### 2. Unión y Envío de Datos (LoRaWAN)
| Comando | Formato Set | Descripción |
| :--- | :--- | :--- |
| `AT+JOIN` | `AT+JOIN=<Mode>` | Une a la red. Mode: `0 (ABP)`, `1 (OTAA)`. |
| `AT+LINKC` | `AT+LINKC` | Añade un *Link Check Request* (piggyback) a la próxima subida (uplink). |
| `AT+SEND` | `AT+SEND=<Port>:<Ack>:<Payload>` | Envía un payload binario (hex) por el puerto App `Port [1..199]`. `Ack`: `0 (unconf)`, `1 (conf)`. |
| `AT+DEVNONCE` | `AT+DEVNONCE=0` | Obtiene o reinicia el DevNonce a 0. |

### 3. Configuración de Red MAc (LoRaWAN)
| Comando | Formato Set | Descripción |
| :--- | :--- | :--- |
| `AT+BAND` | `AT+BAND=<BandID>` | Define Región Activa. `0:AS923, 1:AU915, ... 4:EU433, 5:EU868, ... 8:US915...` |
| `AT+CLASS` | `AT+CLASS=<Class>` | Define Clase del dispositivo. `A`, `B`, o `C`. |
| `AT+ADR` | `AT+ADR=<ADR>` | Adaptive Data Rate (ADR). `0:off`, `1:on`. |
| `AT+DR` | `AT+DR=<DataRate>` | Data Rate de Transmisión (Tx DataRate). `[0..7]`. |
| `AT+DCS` | `AT+DCS=<DutyCycle>` | ETSI DutyCycle (testing). `0:disable`, `1:enable`. |
| `AT+RX1DL` | `AT+RX1DL=<Delay>` | Retardo Ventana Rx 1 en ms. |
| `AT+RX2DL` | `AT+RX2DL=<Delay>` | Retardo Ventana Rx 2 en ms. |
| `AT+RX2DR` | `AT+RX2DR=<DataRate>` | Data Rate de Ventana Rx 2. `[0..7]`. |
| `AT+RX2FQ` | `AT+RX2FQ=<Freq>` | Frecuencia Ventana Rx 2 en Hz. |
| `AT+JN1DL`/`JN2DL`| `AT+JN1DL=<Delay>` | Retardo Ventana Join Accept 1 y 2 en ms. |
| `AT+TXP` | `AT+TXP=<Power>` | Transmit Power `[0..15]` (rango según región). |
| `AT+PGSLOT` | `AT+PGSLOT=<Period>` | Set/Get unicast ping slot Period `[0:1s .. 7:128s]`. |
| `AT+NWKTYPE` | `AT+NWKTYPE=<NetworkType>`| Config. Network Type. `0:Public`, `1:Private`. |
| `AT+CNFRETX` | `AT+CNFRETX=<NbTrans>` | Retransmisiones máximas *Confirmed* `[1..15]`. |
| `AT+UNCNFRETX` | `AT+UNCNFRETX=<NbTrans>`| Retransmisiones máximas *Unconfirmed* `[1..15]`. |
| `AT+CHMASK` | `AT+CHMASK=<Mask>` | Set/Get máscara de canales. |
| `AT+ABPFCNT` | `AT+ABPFCNT=<Fcnt>` | Get/Set el contador de frames de ABP. `[0..4294967295]`. |

### 4. Modo de Pruebas y Certificación (LoRa/RF)
*Nota: Estos son usados generalmente para test RF, certificaciones y hopping.*
| Comando | Descripción |
| :--- | :--- |
| `AT+TTONE` / `AT+TRSSI` | Inicia tono continuo TX / tono de prueba RX RSSI. |
| `AT+TCONF` | Set config RF `<Freq>:<Power>:<BW>:<SF/Dr>:<CR>:<Lna>...`. |
| `AT+TTX` / `AT+TRX` | Inicia envíos de test TX (`AT+TTX=<Packets>`) / test de espera RX (`AT+TRX=<Packets>`). |
| `AT+TTH` | Tx hopping test (`AT+TTH=<Fstart>,<Fstop>,<Fdelta>,<PacketNb>`). |
| `AT+TOFF` | Detiene las pruebas de radio en curso. |
| `AT+CERTIF` | Entra en modo Certificación LoraWan `Mode [0:ABP, 1:OTAA]`. |
| `AT+MTX` / `AT+MRX` / `AT+CERTISEND` | Pruebas de modulación internas y envío cert. |
| `AT+PCONF` / `AT+PSEND` / `AT+PRECV` | Configuración / transmisiones Punto a Punto (P2P / Lora PHY puro). |

---

## Comandos AT: Sigfox

### 1. Variables de Dispositivo e Identidad
| Comando | Formato | Descripción |
| :--- | :--- | :--- |
| `AT$ID` | `AT$ID` | Devuelve el **Sigfox ID** del dispositivo. |
| `AT$PAC` | `AT$PAC` | Devuelve el **PAC** (Porting Authorization Code) inicial. |
| `ATS410` | `ATS410=<Mode>` | `0`: Clave privada, `1`: Clave pública (Public Key). |
| `ATS411` | `ATS411=<Mode>` | Payload Encryption. `0`: Off, `1`: On. |

### 2. Envío de Datos a la Red Sigfox
| Comando | Formato Set | Descripción |
| :--- | :--- | :--- |
| `AT$SB` | `AT$SB=<Bit>,<Downlink>,<TxRepeat>` | Envía un **bit dinámico** de estado a la red Sigfox. |
| `AT$SF` | `AT$SF=<TextoASCII>,<Down>,<TxRpt>` | Envía un **Data frame ASCII** (Ej: `AT$SF=Hola`). |
| `AT$SH` | `AT$SH=<Len>,<HexPayload>,<Dwn>,<Rpt>` | Envía un **Hex frame** (Datos crudos en hex). |
| `ATS300` | `ATS300` | Envía un mensaje *Out of Band* una sola vez. |

### 3. Configuración Regional (Macro-Canales) Mac de Sigfox
| Comando | Formato Set | Descripción |
| :--- | :--- | :--- |
| `AT$RC` | `AT$RC=<Zone>` | Configurar o leer la zona o Región. `Zone = [1 al 7]`. |
| `ATS302` | `ATS302=<Power>` | Setter Radio output power en dBm. `[0..20]`. |
| `ATS400` | `ATS400=<P1><P2><P3>,<P4>` | Variables específicas del estándar (Principalmente FCC). |
| `AT$RSSICAL` | `AT$RSSICAL=<Valor_dB>` | Obtener o establecer el valor de calibración del RSSI en dB. |
| `ATE` | `ATE=<State>` | Estado del Echo sobre UART. `0` = Disable, `1` = Enable. |

### 4. Pruebas y Modos de Scan (Sigfox)
*Nota: Orientado a test y certificaciones P1 / P2 RF.*
| Comando | Formato | Descripción |
| :--- | :--- | :--- |
| `AT$CW` | `AT$CW=<Freq>` | Prueba CW (Continuous Wave) en frecuencia dada Hz o MHz. |
| `AT$PN` | `AT$PN=<Freq>,<Bitrate>` | Inicia test PRBS9 (BPSK pseudo-random test). |
| `AT$MN` | `AT$MN=<TimeOut>` | Inicia escaneo *Monarch* por tiempo indicado (Timeout sec). |
| `AT$TM` | `AT$TM=<rc>,<mode>` | Run Sigfox Test mode, `mode=[0..12]`. |
| `AT$RL` | `AT$RL` | Test: Inicia escucha (listening) de ciclo Local Loop. |
| `AT$SL` | `AT$SL` | Test: Envía paquete Tx de Local Loop. |
| `AT$SP2P` | `AT$SP2P` | Test: Send data for P2P. |
| `AT$RP2P` | `AT$RP2P` | Test: Receive data for P2P. |
