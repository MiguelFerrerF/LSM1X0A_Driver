# Análisis Detallado de Respuestas AT: LoRaWAN

Este documento detalla las respuestas devueltas por el firmware para cada comando AT, incluyendo los mensajes enviados por el puerto serial (AT_PRINTF) y los códigos de error internos que terminan traduciéndose en respuestas como \r\nOK\r\n o \r\nAT_ERROR\r\n.

## Comando / Función: AT_return_ok_l
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_return_error_l
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR

---
## Comando / Función: AT_selection_get_l
### Mensajes por Serial (AT_PRINTF):
- ACTIVE_APP_LORAWAN
- ACTIVE_APP_SIGFOX

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_selection_set_l
### Mensajes por Serial (AT_PRINTF):
- "AT+MODE: LoRa/SigFox Selection error\r\n"
- "Set the Baudrate to 9600\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_fw_set_l
### Mensajes por Serial (AT_PRINTF):
- "FW MODE\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_restore_factory_settings_l
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_store_context
### Mensajes por Serial (AT_PRINTF):
- "NVM DATA UP TO DATE\r\n"
- "NVM DATA STORE FAILED\r\n"

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK

---
## Comando / Función: AT_verbose_get_l
### Mensajes por Serial (AT_PRINTF):
- UTIL_ADV_TRACE_GetVerboseLevel()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_verbose_set_l
### Mensajes por Serial (AT_PRINTF):
- "AT+VL: verbose level is not well set\r\n"
- "AT+VL: verbose level out of range => 0(VLEVEL_OFF) to 3(VLEVEL_H)\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_LocalTime_get
### Mensajes por Serial (AT_PRINTF):
- "LTIME:%02dh%02dm%02ds on %02d/%02d/%04d\r\n", localtime.tm_hour, localtime.tm_min, localtime.tm_sec, localtime.tm_mday, localtime.tm_mon + 1, localtime.tm_year + 1900

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_BaudRate_set
### Mensajes por Serial (AT_PRINTF):
- "Set BaudRate: %d\r\n",baudrate_num
- "Please reboot\r\n"
- "allowed_baudrate: 9600, 115200 \r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_BaudRate_get
### Mensajes por Serial (AT_PRINTF):
- "Set BaudRate: %d\r\n",baudrate_num
- "Please reboot\r\n"
- "Not set BaudRate - default: 9600\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_JoinEUI_get
### Mensajes por Serial (AT_PRINTF):
- appEUI

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_JoinEUI_set
### Mensajes por Serial (AT_PRINTF):
- "DevNonce initialization\r\n"

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_NwkKey_get
### Mensajes por Serial (AT_PRINTF):
- nwkKey

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK

---
## Comando / Función: AT_NwkKey_set
### Mensajes por Serial (AT_PRINTF):
- "DevNonce initialization\r\n"

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_AppKey_get
### Mensajes por Serial (AT_PRINTF):
- appKey

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK

---
## Comando / Función: AT_AppKey_set
### Mensajes por Serial (AT_PRINTF):
- "DevNonce initialization\r\n"

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_NwkSKey_get
### Mensajes por Serial (AT_PRINTF):
- nwkSKey

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK

---
## Comando / Función: AT_NwkSKey_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_AppSKey_get
### Mensajes por Serial (AT_PRINTF):
- appSKey

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK

---
## Comando / Función: AT_AppSKey_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DevAddr_get
### Mensajes por Serial (AT_PRINTF):
- devAddr

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DevAddr_set
### Mensajes por Serial (AT_PRINTF):
- "Upfcnt not initialization"

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DevEUI_get
### Mensajes por Serial (AT_PRINTF):
- devEUI

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DevEUI_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_NetworkID_get
### Mensajes por Serial (AT_PRINTF):
- networkId

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_NetworkID_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Join_get
### Mensajes por Serial (AT_PRINTF):
- E2P_LORA_Read_Type()-1

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Join
### Mensajes por Serial (AT_PRINTF):
- "ABP mode\r\n"
- "OTAA mode\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Link_Check
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Send
### Mensajes por Serial (AT_PRINTF):
- "AT+SEND without the application port\r\n"
- "AT+SEND missing : character after app port\r\n"
- "AT+SEND without the acknowledge flag\r\n"
- "AT+SEND missing : character after ack flag\r\n"

### Valores de Retorno (Códigos AT):
- AT_PARAM_ERROR

---
## Comando / Función: AT_version_get_l
### Mensajes por Serial (AT_PRINTF):
- "APP_VERSION: V%X.%X.%X\r\n", (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT)
- "MW_LORAWAN_VERSION: V%X.%X.%X\r\n", (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT)
- "MW_RADIO_VERSION: V%X.%X.%X\r\n", (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB2_SHIFT)

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_ADR_get
### Mensajes por Serial (AT_PRINTF):
- adrEnable

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_ADR_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DataRate_get
### Mensajes por Serial (AT_PRINTF):
- txDatarate

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DataRate_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Region_get
### Mensajes por Serial (AT_PRINTF):
- "%d:%s\r\n", region, regionStrings[region]

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Region_set
### Mensajes por Serial (AT_PRINTF):
- "sub_band_AS923_1: %d \r\n",AS923_sub_band
- "LORAMAC_HANDLER_SUCCESS \r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DeviceClass_get
### Mensajes por Serial (AT_PRINTF):
- "B,S0\r\n"
- "B,S1\r\n"
- "B,S2\r\n"
- "%c\r\n", 'A' + currentClass

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DeviceClass_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_NO_CLASS_B_ENABLE
- AT_ERROR
- AT_OK
- AT_NO_NET_JOINED
- AT_PARAM_ERROR

---
## Comando / Función: AT_DutyCycle_get
### Mensajes por Serial (AT_PRINTF):
- dutyCycleEnable

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DutyCycle_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_JoinAcceptDelay1_get
### Mensajes por Serial (AT_PRINTF):
- rxDelay

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_JoinAcceptDelay1_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_JoinAcceptDelay2_get
### Mensajes por Serial (AT_PRINTF):
- rxDelay

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_JoinAcceptDelay2_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx1Delay_get
### Mensajes por Serial (AT_PRINTF):
- rxDelay

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx1Delay_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx2Delay_get
### Mensajes por Serial (AT_PRINTF):
- rxDelay

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx2Delay_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx2DataRate_get
### Mensajes por Serial (AT_PRINTF):
- rx2Params.Datarate

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Rx2DataRate_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Rx2Frequency_get
### Mensajes por Serial (AT_PRINTF):
- rx2Params.Frequency

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Rx2Frequency_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_TransmitPower_get
### Mensajes por Serial (AT_PRINTF):
- txPower

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_TransmitPower_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_PingSlot_get
### Mensajes por Serial (AT_PRINTF):
- periodicity

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_PingSlot_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Network_Type_get
### Mensajes por Serial (AT_PRINTF):
- "Private Mode\r\n"
- "Public Mode\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Network_Type_set
### Mensajes por Serial (AT_PRINTF):
- "Public Mode\r\n"
- "Private Mode\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_DevNonce_get
### Mensajes por Serial (AT_PRINTF):
- nonce

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_DevNonce_set
### Mensajes por Serial (AT_PRINTF):
- "DevNonce initialization\r\n"
- nonce

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_ABP_Fcnt_get
### Mensajes por Serial (AT_PRINTF):
- Fcnt

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_ABP_Fcnt_set
### Mensajes por Serial (AT_PRINTF):
- Fcnt

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Confirmed_Retransmission_get
### Mensajes por Serial (AT_PRINTF):
- NbTrans

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Confirmed_Retransmission_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Unconfirmed_Retransmission_get
### Mensajes por Serial (AT_PRINTF):
- NbTrans

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Unconfirmed_Retransmission_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Channel_Mask_set
### Mensajes por Serial (AT_PRINTF):
- "KR920 command"
- "Error: Unusable channel mask is set\r\n"
- "info: maximum channels number: %d\r\n",REGION_NVM_MAX_NB_CHANNELS
- "channel_mask[%d]: 0x%04x\r\n",i,enable_chmask[i]

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_NO_NET_JOINED
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Channel_Mask_get
### Mensajes por Serial (AT_PRINTF):
- "channel_mask[%d]: 0x%04x\r\n",i,enable_chmask[i]

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_txTone
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR

---
## Comando / Función: AT_test_rxRssi
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR

---
## Comando / Función: AT_test_get_config
### Mensajes por Serial (AT_PRINTF):
- "1: Freq= %d Hz\r\n", testParam.freq
- "2: Power= %d dBm\r\n", testParam.power
- "3: Bandwidth= %d Hz\r\n", testParam.bandwidth
- "4: FSK datarate= %d bps\r\n", testParam.loraSf_datarate
- "5: Coding Rate not applicable\r\n"
- "6: LNA State= %d \r\n", testParam.lna
- "7: PA Boost State= %d \r\n", testParam.paBoost
- "8: modulation FSK\r\n"
- "8: modulation MSK\r\n"
- "9: Payload len= %d Bytes\r\n", testParam.payloadLen
- "10: FSK deviation= %d Hz\r\n", testParam.fskDev
- "10: FSK deviation forced to FSK datarate/4\r\n"
- "11: LowDRopt not applicable\r\n"
- "12: FSK gaussian BT product= %d \r\n", testParam.BTproduct
- "3: Bandwidth= %d (=%d Hz)\r\n", testParam.bandwidth, loraBW[testParam.bandwidth]
- "4: SF= %d \r\n", testParam.loraSf_datarate
- "5: CR= %d (=4/%d) \r\n", testParam.codingRate, testParam.codingRate + 4
- "6: LNA State= %d \r\n", testParam.lna
- "7: PA Boost State= %d \r\n", testParam.paBoost
- "8: modulation LORA\r\n"
- "9: Payload len= %d Bytes\r\n", testParam.payloadLen
- "10: Frequency deviation not applicable\r\n"
- "11: LowDRopt[0 to 2]= %d \r\n", testParam.lowDrOpt
- "12 BT product not applicable\r\n"
- "4: BPSK datarate= %d bps\r\n", testParam.loraSf_datarate
- "can be copy/paste in set cmd: AT+TCONF=%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d:%d:%d\r\n", testParam.freq, testParam.power, testParam.bandwidth, testParam.loraSf_datarate, testParam.codingRate + 4, \ testParam.lna, testParam.paBoost, testParam.modulation, testParam.payloadLen, testParam.fskDev, testParam.lowDrOpt, testParam.BTproduct

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_test_set_config
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_tx
### Mensajes por Serial (AT_PRINTF):
- "\r\nTTxStart\r\n"
- "AT+TTX: nb packets sent is missing\r\n"
- "\r\nTTxEnd\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_rx
### Mensajes por Serial (AT_PRINTF):
- "TRxStart\r\n"
- "AT+TRX: nb expected packets is missing\r\n"
- "TRxEnd\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_Certif
### Mensajes por Serial (AT_PRINTF):
- "ABP mode\r\n"
- "OTAA mode\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_tx_hopping
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_stop
### Mensajes por Serial (AT_PRINTF):
- "Test Stop\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_test_Modulation_Tx
### Mensajes por Serial (AT_PRINTF):
- "\r\nTx Modulation TEST START\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR

---
## Comando / Función: AT_test_Modulation_Rx
### Mensajes por Serial (AT_PRINTF):
- "\r\nRx Modulation TEST START\r\n"
- "\r\nRx Modulation TEST STOP\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR

---
## Comando / Función: AT_P2P_get_config
### Mensajes por Serial (AT_PRINTF):
- "1: Freq= %d Hz\r\n", P2PParam.freq
- "2: Power= %d dBm\r\n", P2PParam.power
- "3: Bandwidth= %d Hz\r\n", P2PParam.bandwidth
- "4: FSK datarate= %d bps\r\n", P2PParam.loraSf_datarate
- "5: Coding Rate not applicable\r\n"
- "6: LNA State= %d \r\n", P2PParam.lna
- "7: PA Boost State= %d \r\n", P2PParam.paBoost
- "8: modulation FSK\r\n"
- "8: modulation MSK\r\n"
- "9: Payload len= %d Bytes\r\n", P2PParam.payloadLen
- "10: FSK deviation= %d Hz\r\n", P2PParam.fskDev
- "10: FSK deviation forced to FSK datarate/4\r\n"
- "11: LowDRopt not applicable\r\n"
- "12: FSK gaussian BT product= %d \r\n", P2PParam.BTproduct
- "3: Bandwidth= %d (=%d Hz)\r\n", P2PParam.bandwidth, loraBW[P2PParam.bandwidth]
- "4: SF= %d \r\n", P2PParam.loraSf_datarate
- "5: CR= %d (=4/%d) \r\n", P2PParam.codingRate, P2PParam.codingRate + 4
- "6: LNA State= %d \r\n", P2PParam.lna
- "7: PA Boost State= %d \r\n", P2PParam.paBoost
- "8: modulation LORA\r\n"
- "9: Payload len= %d Bytes\r\n", P2PParam.payloadLen
- "10: Frequency deviation not applicable\r\n"
- "11: LowDRopt[0 to 2]= %d \r\n", P2PParam.lowDrOpt
- "12 BT product not applicable\r\n"
- "4: BPSK datarate= %d bps\r\n", P2PParam.loraSf_datarate
- "can be copy/paste in set cmd: AT+TCONF=%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d:%d:%d\r\n", P2PParam.freq, P2PParam.power, P2PParam.bandwidth, P2PParam.loraSf_datarate, P2PParam.codingRate + 4, \ P2PParam.lna, P2PParam.paBoost, P2PParam.modulation, P2PParam.payloadLen, P2PParam.fskDev, P2PParam.lowDrOpt, P2PParam.BTproduct

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_P2P_set_config
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_P2P_Tx
### Mensajes por Serial (AT_PRINTF):
- "\r\nP2P Tx START\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_P2P_Rx
### Mensajes por Serial (AT_PRINTF):
- "\r\nP2P Rx START\r\n"
- "\r\nP2P Rx STOP\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_BUSY_ERROR

---
## Comando / Función: AT_write_register
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_read_register
### Mensajes por Serial (AT_PRINTF):
- "REG 0x%04X=0x%02X", add16, data

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_bat_get_l
### Mensajes por Serial (AT_PRINTF):
- SYS_GetBatteryLevel()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_sw_version_get_l
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
