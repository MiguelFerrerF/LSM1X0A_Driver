# Análisis Detallado de Respuestas AT: Sigfox

Este documento detalla las respuestas devueltas por el firmware para cada comando AT, incluyendo los mensajes enviados por el puerto serial (AT_PRINTF) y los códigos de error internos que terminan traduciéndose en respuestas como \r\nOK\r\n o \r\nAT_ERROR\r\n.

## Comando / Función: AT_return_ok
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_return_error
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR

---
## Comando / Función: AT_selection_get
### Mensajes por Serial (AT_PRINTF):
- ACTIVE_APP_LORAWAN
- ACTIVE_APP_SIGFOX

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_selection_set
### Mensajes por Serial (AT_PRINTF):
- ">> AT_selection_set 1 \r\n"
- "AT+MODE: LoRa/SigFox Selection error\r\n"
- ">> selection Set %u \r\n", selection

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_fw_set
### Mensajes por Serial (AT_PRINTF):
- "FW MODE\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_restore_factory_settings
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_SendBit
### Mensajes por Serial (AT_PRINTF):
- dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

### Valores de Retorno (Códigos AT):
- AT_TX_TIMEOUT
- AT_LIB_ERROR
- AT_RX_TIMEOUT
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_SendFrame
### Mensajes por Serial (AT_PRINTF):
- dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

### Valores de Retorno (Códigos AT):
- AT_TX_TIMEOUT
- AT_LIB_ERROR
- AT_RX_TIMEOUT
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_SendHexFrame
### Mensajes por Serial (AT_PRINTF):
- dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

### Valores de Retorno (Códigos AT):
- AT_TX_TIMEOUT
- AT_LIB_ERROR
- AT_RX_TIMEOUT
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_version_get
### Mensajes por Serial (AT_PRINTF):
- version, size
- "\r\n"
- version, size
- "\r\n"
- version, size
- "\r\n"
- version, size
- "\r\n"
- version, size
- "\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_DevPac_get
### Mensajes por Serial (AT_PRINTF):
- "%02X", SfxPac[i]
- "\r\n"

### Valores de Retorno (Códigos AT):
- AT_LIB_ERROR
- AT_OK

---
## Comando / Función: AT_DevId_get
### Mensajes por Serial (AT_PRINTF):
- "%02X", SfxId[i - 1]
- "\r\n"

### Valores de Retorno (Códigos AT):
- AT_LIB_ERROR
- AT_OK

---
## Comando / Función: AT_PublicKey_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_PublicKey_get
### Mensajes por Serial (AT_PRINTF):
- "%d\r\n", (uint8_t) E2P_Read_KeyType()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_PayloadEncryption_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_PayloadEncryption_get
### Mensajes por Serial (AT_PRINTF):
- "%d\r\n", (uint8_t) E2P_Read_EncryptionFlag()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_bat_get
### Mensajes por Serial (AT_PRINTF):
- SYS_GetBatteryLevel()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_test_cw
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_RECONF_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_pn
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_RECONF_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_scan_mn
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_mode
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_PARAM_ERROR

---
## Comando / Función: AT_power_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_power_get
### Mensajes por Serial (AT_PRINTF):
- "%d\r\n", E2P_Read_Power(E2P_Read_Rc())

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_outOfBand_run
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_ChannelConfigFcc_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_rc_get
### Mensajes por Serial (AT_PRINTF):
- "1\r\n"
- "2\r\n"
- "3A\r\n"
- "3C\r\n"
- "4\r\n"
- "5\r\n"
- "6\r\n"
- "7\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_rc_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_ERROR
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_rssi_cal_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_rssi_cal_get
### Mensajes por Serial (AT_PRINTF):
- "%d dB\r\n", rssi_cal

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_echo_set
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_echo_get
### Mensajes por Serial (AT_PRINTF):
- "%d\r\n", echoState

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_verbose_set
### Mensajes por Serial (AT_PRINTF):
- "AT+VL: verbose level is not well set"
- "AT+VL: verbose level out of range => 0(VLEVEL_OFF) to 3(VLEVEL_H)"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_verbose_get
### Mensajes por Serial (AT_PRINTF):
- UTIL_ADV_TRACE_GetVerboseLevel()

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_test_rl
### Mensajes por Serial (AT_PRINTF):
- "\r\nRX_TEST_START\r\n"
- "\r\nOK\r\n"
- "RF_API_wait_frame : result[%x] state[%d]\r\n\r\n", result, state
- "RL="
- "%02X ", frame[j]
- "\r\n"
- "{#%d RSSI=%d TEST PASSED!}\r\n\r\n",receivedCnt,rssi
- "RF_API_wait_frame : result[%x] state[%d] - TIMEOUT!!\r\n\r\n", result, state
- "\r\nRX_TEST_STOP\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_RECONF_ERROR
- AT_PARAM_ERROR

---
## Comando / Función: AT_test_sl
### Mensajes por Serial (AT_PRINTF):
- "FREP[%u] dataRate[%u]\r\n", freq, dataRate

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_sw_version_get
### Mensajes por Serial: Ninguno adicional.

### Valores de Retorno (Códigos AT):
- AT_OK

---
## Comando / Función: AT_Send_P2P_Data
### Mensajes por Serial (AT_PRINTF):
- "Send DATA.....\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK
- AT_PARAM_ERROR

---
## Comando / Función: AT_Receive_P2P_Data
### Mensajes por Serial (AT_PRINTF):
- "\r\nOK\r\n"
- "\r\nRecevie DATA.... START\r\n"
- "RF_API_wait_frame : result[%x] state[%d]\r\n\r\n", result, state
- "RP2P="
- "%02X ", frame[j]
- "\r\n"
- "{#%d RSSI=%d RECEIVED!!}\r\n\r\n",receivedCnt,rssi
- "RF_API_wait_frame : result[%x] state[%d] - TIMEOUT!!\r\n\r\n", result, state
- "\r\nRecevie DATA.... STOP\r\n"

### Valores de Retorno (Códigos AT):
- AT_OK

---
