\page lsm1x0a_sigfox_responses_page AT Responses: Sigfox

# Detailed AT Responses Analysis: Sigfox

This document details the responses returned by the firmware for each AT command, including messages sent via the serial port (`AT_PRINTF`) and the internal error codes that ultimately translate into responses like `\r\nOK\r\n` or `\r\nAT_ERROR\r\n`.

## `AT_return_ok`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_return_error`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`

---

## `AT_selection_get`
**Serial Messages (`AT_PRINTF`):**
```text
ACTIVE_APP_LORAWAN
ACTIVE_APP_SIGFOX

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_selection_set`
**Serial Messages (`AT_PRINTF`):**
```text
>> AT_selection_set 1 \r\n
AT+MODE: LoRa/SigFox Selection error\r\n
">> selection Set %u \r\n", selection

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_fw_set`
**Serial Messages (`AT_PRINTF`):**
```text
FW MODE\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_restore_factory_settings`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_SendBit`
**Serial Messages (`AT_PRINTF`):**
```text
dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

```

**Return Values (AT Codes):**
- `AT_TX_TIMEOUT`
- `AT_LIB_ERROR`
- `AT_RX_TIMEOUT`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_SendFrame`
**Serial Messages (`AT_PRINTF`):**
```text
dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

```

**Return Values (AT Codes):**
- `AT_TX_TIMEOUT`
- `AT_LIB_ERROR`
- `AT_RX_TIMEOUT`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_SendHexFrame`
**Serial Messages (`AT_PRINTF`):**
```text
dl_msg, SGFX_MAX_DL_PAYLOAD_SIZE

```

**Return Values (AT Codes):**
- `AT_TX_TIMEOUT`
- `AT_LIB_ERROR`
- `AT_RX_TIMEOUT`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_version_get`
**Serial Messages (`AT_PRINTF`):**
```text
version, size
\r\n
version, size
\r\n
version, size
\r\n
version, size
\r\n
version, size
\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_DevPac_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%02X", SfxPac[i]
\r\n

```

**Return Values (AT Codes):**
- `AT_LIB_ERROR`
- `AT_OK`

---

## `AT_DevId_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%02X", SfxId[i - 1]
\r\n

```

**Return Values (AT Codes):**
- `AT_LIB_ERROR`
- `AT_OK`

---

## `AT_PublicKey_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_PublicKey_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d\r\n", (uint8_t) E2P_Read_KeyType()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_PayloadEncryption_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_PayloadEncryption_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d\r\n", (uint8_t) E2P_Read_EncryptionFlag()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_bat_get`
**Serial Messages (`AT_PRINTF`):**
```text
SYS_GetBatteryLevel()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_test_cw`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_RECONF_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_test_pn`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_RECONF_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_scan_mn`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_test_mode`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_PARAM_ERROR`

---

## `AT_power_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_power_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d\r\n", E2P_Read_Power(E2P_Read_Rc())

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_outOfBand_run`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_ChannelConfigFcc_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_rc_get`
**Serial Messages (`AT_PRINTF`):**
```text
1\r\n
2\r\n
3A\r\n
3C\r\n
4\r\n
5\r\n
6\r\n
7\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_rc_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_rssi_cal_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_rssi_cal_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d dB\r\n", rssi_cal

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_echo_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_echo_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d\r\n", echoState

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_verbose_set`
**Serial Messages (`AT_PRINTF`):**
```text
AT+VL: verbose level is not well set
AT+VL: verbose level out of range => 0(VLEVEL_OFF) to 3(VLEVEL_H)

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_verbose_get`
**Serial Messages (`AT_PRINTF`):**
```text
UTIL_ADV_TRACE_GetVerboseLevel()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_test_rl`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nRX_TEST_START\r\n
\r\nOK\r\n
"RF_API_wait_frame : result[%x] state[%d]\r\n\r\n", result, state
RL=
"%02X ", frame[j]
\r\n
"{#%d RSSI=%d TEST PASSED!}\r\n\r\n",receivedCnt,rssi
"RF_API_wait_frame : result[%x] state[%d] - TIMEOUT!!\r\n\r\n", result, state
\r\nRX_TEST_STOP\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_RECONF_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_test_sl`
**Serial Messages (`AT_PRINTF`):**
```text
"FREP[%u] dataRate[%u]\r\n", freq, dataRate

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_sw_version_get`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Send_P2P_Data`
**Serial Messages (`AT_PRINTF`):**
```text
Send DATA.....\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Receive_P2P_Data`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nOK\r\n
\r\nRecevie DATA.... START\r\n
"RF_API_wait_frame : result[%x] state[%d]\r\n\r\n", result, state
RP2P=
"%02X ", frame[j]
\r\n
"{#%d RSSI=%d RECEIVED!!}\r\n\r\n",receivedCnt,rssi
"RF_API_wait_frame : result[%x] state[%d] - TIMEOUT!!\r\n\r\n", result, state
\r\nRecevie DATA.... STOP\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

