# Detailed AT Responses Analysis: LoRaWAN

This document details the responses returned by the firmware for each AT command, including messages sent via the serial port (`AT_PRINTF`) and the internal error codes that ultimately translate into responses like `\r\nOK\r\n` or `\r\nAT_ERROR\r\n`.

## `AT_return_ok_l`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_return_error_l`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`

---

## `AT_selection_get_l`
**Serial Messages (`AT_PRINTF`):**
```text
ACTIVE_APP_LORAWAN
ACTIVE_APP_SIGFOX

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_selection_set_l`
**Serial Messages (`AT_PRINTF`):**
```text
AT+MODE: LoRa/SigFox Selection error\r\n
Set the Baudrate to 9600\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_fw_set_l`
**Serial Messages (`AT_PRINTF`):**
```text
FW MODE\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_restore_factory_settings_l`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_store_context`
**Serial Messages (`AT_PRINTF`):**
```text
NVM DATA UP TO DATE\r\n
NVM DATA STORE FAILED\r\n

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`

---

## `AT_verbose_get_l`
**Serial Messages (`AT_PRINTF`):**
```text
UTIL_ADV_TRACE_GetVerboseLevel()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_verbose_set_l`
**Serial Messages (`AT_PRINTF`):**
```text
AT+VL: verbose level is not well set\r\n
AT+VL: verbose level out of range => 0(VLEVEL_OFF) to 3(VLEVEL_H)\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_LocalTime_get`
**Serial Messages (`AT_PRINTF`):**
```text
"LTIME:%02dh%02dm%02ds on %02d/%02d/%04d\r\n", localtime.tm_hour, localtime.tm_min, localtime.tm_sec, localtime.tm_mday, localtime.tm_mon + 1, localtime.tm_year + 1900

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_BaudRate_set`
**Serial Messages (`AT_PRINTF`):**
```text
"Set BaudRate: %d\r\n",baudrate_num
Please reboot\r\n
allowed_baudrate: 9600, 115200 \r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_BaudRate_get`
**Serial Messages (`AT_PRINTF`):**
```text
"Set BaudRate: %d\r\n",baudrate_num
Please reboot\r\n
Not set BaudRate - default: 9600\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_JoinEUI_get`
**Serial Messages (`AT_PRINTF`):**
```text
appEUI

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_JoinEUI_set`
**Serial Messages (`AT_PRINTF`):**
```text
DevNonce initialization\r\n

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_NwkKey_get`
**Serial Messages (`AT_PRINTF`):**
```text
nwkKey

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`

---

## `AT_NwkKey_set`
**Serial Messages (`AT_PRINTF`):**
```text
DevNonce initialization\r\n

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_AppKey_get`
**Serial Messages (`AT_PRINTF`):**
```text
appKey

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`

---

## `AT_AppKey_set`
**Serial Messages (`AT_PRINTF`):**
```text
DevNonce initialization\r\n

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_NwkSKey_get`
**Serial Messages (`AT_PRINTF`):**
```text
nwkSKey

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`

---

## `AT_NwkSKey_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_AppSKey_get`
**Serial Messages (`AT_PRINTF`):**
```text
appSKey

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`

---

## `AT_AppSKey_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DevAddr_get`
**Serial Messages (`AT_PRINTF`):**
```text
devAddr

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DevAddr_set`
**Serial Messages (`AT_PRINTF`):**
```text
Upfcnt not initialization

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DevEUI_get`
**Serial Messages (`AT_PRINTF`):**
```text
devEUI

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DevEUI_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_NetworkID_get`
**Serial Messages (`AT_PRINTF`):**
```text
networkId

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_NetworkID_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Join_get`
**Serial Messages (`AT_PRINTF`):**
```text
E2P_LORA_Read_Type()-1

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Join`
**Serial Messages (`AT_PRINTF`):**
```text
ABP mode\r\n
OTAA mode\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Link_Check`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Send`
**Serial Messages (`AT_PRINTF`):**
```text
AT+SEND without the application port\r\n
AT+SEND missing : character after app port\r\n
AT+SEND without the acknowledge flag\r\n
AT+SEND missing : character after ack flag\r\n

```

**Return Values (AT Codes):**
- `AT_PARAM_ERROR`

---

## `AT_version_get_l`
**Serial Messages (`AT_PRINTF`):**
```text
"APP_VERSION: V%X.%X.%X\r\n", (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT)
"MW_LORAWAN_VERSION: V%X.%X.%X\r\n", (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT)
"MW_RADIO_VERSION: V%X.%X.%X\r\n", (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_MAIN_SHIFT), (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB1_SHIFT), (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB2_SHIFT)

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_ADR_get`
**Serial Messages (`AT_PRINTF`):**
```text
adrEnable

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_ADR_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DataRate_get`
**Serial Messages (`AT_PRINTF`):**
```text
txDatarate

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DataRate_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Region_get`
**Serial Messages (`AT_PRINTF`):**
```text
"%d:%s\r\n", region, regionStrings[region]

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Region_set`
**Serial Messages (`AT_PRINTF`):**
```text
"sub_band_AS923_1: %d \r\n",AS923_sub_band
LORAMAC_HANDLER_SUCCESS \r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DeviceClass_get`
**Serial Messages (`AT_PRINTF`):**
```text
B,S0\r\n
B,S1\r\n
B,S2\r\n
"%c\r\n", 'A' + currentClass

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DeviceClass_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_NO_CLASS_B_ENABLE`
- `AT_ERROR`
- `AT_OK`
- `AT_NO_NET_JOINED`
- `AT_PARAM_ERROR`

---

## `AT_DutyCycle_get`
**Serial Messages (`AT_PRINTF`):**
```text
dutyCycleEnable

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DutyCycle_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_JoinAcceptDelay1_get`
**Serial Messages (`AT_PRINTF`):**
```text
rxDelay

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_JoinAcceptDelay1_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_JoinAcceptDelay2_get`
**Serial Messages (`AT_PRINTF`):**
```text
rxDelay

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_JoinAcceptDelay2_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx1Delay_get`
**Serial Messages (`AT_PRINTF`):**
```text
rxDelay

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx1Delay_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx2Delay_get`
**Serial Messages (`AT_PRINTF`):**
```text
rxDelay

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx2Delay_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx2DataRate_get`
**Serial Messages (`AT_PRINTF`):**
```text
rx2Params.Datarate

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Rx2DataRate_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Rx2Frequency_get`
**Serial Messages (`AT_PRINTF`):**
```text
rx2Params.Frequency

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Rx2Frequency_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_TransmitPower_get`
**Serial Messages (`AT_PRINTF`):**
```text
txPower

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_TransmitPower_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_PingSlot_get`
**Serial Messages (`AT_PRINTF`):**
```text
periodicity

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_PingSlot_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Network_Type_get`
**Serial Messages (`AT_PRINTF`):**
```text
Private Mode\r\n
Public Mode\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Network_Type_set`
**Serial Messages (`AT_PRINTF`):**
```text
Public Mode\r\n
Private Mode\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_DevNonce_get`
**Serial Messages (`AT_PRINTF`):**
```text
nonce

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_DevNonce_set`
**Serial Messages (`AT_PRINTF`):**
```text
DevNonce initialization\r\n
nonce

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_ABP_Fcnt_get`
**Serial Messages (`AT_PRINTF`):**
```text
Fcnt

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_ABP_Fcnt_set`
**Serial Messages (`AT_PRINTF`):**
```text
Fcnt

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Confirmed_Retransmission_get`
**Serial Messages (`AT_PRINTF`):**
```text
NbTrans

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Confirmed_Retransmission_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Unconfirmed_Retransmission_get`
**Serial Messages (`AT_PRINTF`):**
```text
NbTrans

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_Unconfirmed_Retransmission_set`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Channel_Mask_set`
**Serial Messages (`AT_PRINTF`):**
```text
KR920 command
Error: Unusable channel mask is set\r\n
"info: maximum channels number: %d\r\n",REGION_NVM_MAX_NB_CHANNELS
"channel_mask[%d]: 0x%04x\r\n",i,enable_chmask[i]

```

**Return Values (AT Codes):**
- `AT_ERROR`
- `AT_NO_NET_JOINED`
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_Channel_Mask_get`
**Serial Messages (`AT_PRINTF`):**
```text
"channel_mask[%d]: 0x%04x\r\n",i,enable_chmask[i]

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_test_txTone`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`

---

## `AT_test_rxRssi`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`

---

## `AT_test_get_config`
**Serial Messages (`AT_PRINTF`):**
```text
"1: Freq= %d Hz\r\n", testParam.freq
"2: Power= %d dBm\r\n", testParam.power
"3: Bandwidth= %d Hz\r\n", testParam.bandwidth
"4: FSK datarate= %d bps\r\n", testParam.loraSf_datarate
5: Coding Rate not applicable\r\n
"6: LNA State= %d \r\n", testParam.lna
"7: PA Boost State= %d \r\n", testParam.paBoost
8: modulation FSK\r\n
8: modulation MSK\r\n
"9: Payload len= %d Bytes\r\n", testParam.payloadLen
"10: FSK deviation= %d Hz\r\n", testParam.fskDev
10: FSK deviation forced to FSK datarate/4\r\n
11: LowDRopt not applicable\r\n
"12: FSK gaussian BT product= %d \r\n", testParam.BTproduct
"3: Bandwidth= %d (=%d Hz)\r\n", testParam.bandwidth, loraBW[testParam.bandwidth]
"4: SF= %d \r\n", testParam.loraSf_datarate
"5: CR= %d (=4/%d) \r\n", testParam.codingRate, testParam.codingRate + 4
"6: LNA State= %d \r\n", testParam.lna
"7: PA Boost State= %d \r\n", testParam.paBoost
8: modulation LORA\r\n
"9: Payload len= %d Bytes\r\n", testParam.payloadLen
10: Frequency deviation not applicable\r\n
"11: LowDRopt[0 to 2]= %d \r\n", testParam.lowDrOpt
12 BT product not applicable\r\n
"4: BPSK datarate= %d bps\r\n", testParam.loraSf_datarate
"can be copy/paste in set cmd: AT+TCONF=%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d:%d:%d\r\n", testParam.freq, testParam.power, testParam.bandwidth, testParam.loraSf_datarate, testParam.codingRate + 4, \ testParam.lna, testParam.paBoost, testParam.modulation, testParam.payloadLen, testParam.fskDev, testParam.lowDrOpt, testParam.BTproduct

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_test_set_config`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_test_tx`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nTTxStart\r\n
AT+TTX: nb packets sent is missing\r\n
\r\nTTxEnd\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_test_rx`
**Serial Messages (`AT_PRINTF`):**
```text
TRxStart\r\n
AT+TRX: nb expected packets is missing\r\n
TRxEnd\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_Certif`
**Serial Messages (`AT_PRINTF`):**
```text
ABP mode\r\n
OTAA mode\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_test_tx_hopping`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_test_stop`
**Serial Messages (`AT_PRINTF`):**
```text
Test Stop\r\n

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_test_Modulation_Tx`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nTx Modulation TEST START\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`

---

## `AT_test_Modulation_Rx`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nRx Modulation TEST START\r\n
\r\nRx Modulation TEST STOP\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`

---

## `AT_P2P_get_config`
**Serial Messages (`AT_PRINTF`):**
```text
"1: Freq= %d Hz\r\n", P2PParam.freq
"2: Power= %d dBm\r\n", P2PParam.power
"3: Bandwidth= %d Hz\r\n", P2PParam.bandwidth
"4: FSK datarate= %d bps\r\n", P2PParam.loraSf_datarate
5: Coding Rate not applicable\r\n
"6: LNA State= %d \r\n", P2PParam.lna
"7: PA Boost State= %d \r\n", P2PParam.paBoost
8: modulation FSK\r\n
8: modulation MSK\r\n
"9: Payload len= %d Bytes\r\n", P2PParam.payloadLen
"10: FSK deviation= %d Hz\r\n", P2PParam.fskDev
10: FSK deviation forced to FSK datarate/4\r\n
11: LowDRopt not applicable\r\n
"12: FSK gaussian BT product= %d \r\n", P2PParam.BTproduct
"3: Bandwidth= %d (=%d Hz)\r\n", P2PParam.bandwidth, loraBW[P2PParam.bandwidth]
"4: SF= %d \r\n", P2PParam.loraSf_datarate
"5: CR= %d (=4/%d) \r\n", P2PParam.codingRate, P2PParam.codingRate + 4
"6: LNA State= %d \r\n", P2PParam.lna
"7: PA Boost State= %d \r\n", P2PParam.paBoost
8: modulation LORA\r\n
"9: Payload len= %d Bytes\r\n", P2PParam.payloadLen
10: Frequency deviation not applicable\r\n
"11: LowDRopt[0 to 2]= %d \r\n", P2PParam.lowDrOpt
12 BT product not applicable\r\n
"4: BPSK datarate= %d bps\r\n", P2PParam.loraSf_datarate
"can be copy/paste in set cmd: AT+TCONF=%d:%d:%d:%d:4/%d:%d:%d:%d:%d:%d:%d:%d\r\n", P2PParam.freq, P2PParam.power, P2PParam.bandwidth, P2PParam.loraSf_datarate, P2PParam.codingRate + 4, \ P2PParam.lna, P2PParam.paBoost, P2PParam.modulation, P2PParam.payloadLen, P2PParam.fskDev, P2PParam.lowDrOpt, P2PParam.BTproduct

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_P2P_set_config`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_P2P_Tx`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nP2P Tx START\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`
- `AT_PARAM_ERROR`

---

## `AT_P2P_Rx`
**Serial Messages (`AT_PRINTF`):**
```text
\r\nP2P Rx START\r\n
\r\nP2P Rx STOP\r\n

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_BUSY_ERROR`

---

## `AT_write_register`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_read_register`
**Serial Messages (`AT_PRINTF`):**
```text
"REG 0x%04X=0x%02X", add16, data

```

**Return Values (AT Codes):**
- `AT_OK`
- `AT_PARAM_ERROR`

---

## `AT_bat_get_l`
**Serial Messages (`AT_PRINTF`):**
```text
SYS_GetBatteryLevel()

```

**Return Values (AT Codes):**
- `AT_OK`

---

## `AT_sw_version_get_l`
**Serial Messages:** None additional.

**Return Values (AT Codes):**
- `AT_OK`

---

