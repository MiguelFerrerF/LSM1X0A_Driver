# Channel Mask Analysis

Based on the source code of the main application (`LSM1x0A_FW/LoRaWAN/App`) and the hardware, here we detail how the module's firmware processes and handles bands (Regions), subbands, and the _Channel Mask_ as implemented through its AT command interface.

The _MiddleWare_ (LoRaWAN Stack and internal LoraMac-Node definitions like `RegionAS923.h` or `LmHandler`) is precompiled or outside the project directory, but direct interaction with it takes place in `lora_at.c` and `lora_command.c`.

## 1. Bands Configuration (Regions)
The main frequency/band is manipulated with the **`AT+BAND`** command.

- **Command**: `AT+BAND=<BandID>[,<SubBand>]`
- **Possible values (`BandID`)**:
  - `0`: AS923
  - `1`: AU915
  - `2`: CN470
  - `3`: CN779
  - `4`: EU433
  - `5`: EU868
  - `6`: KR920
  - `7`: IN865
  - `8`: US915
  - `9`: RU864

### Fixed Subbands by Region (AS923 Case)
The `AT_Region_set` function in `lora_at.c` (line 1306) waits for up to two comma-separated parameters. Specifically, for the **AS923** region, a **subband** must be indicated explicitly:
```c
parameter_num = tiny_sscanf(param, "%hhu,%hhu", &region, &AS923_sub_band);
if(region == LORAMAC_REGION_AS923) {
    if(AS923_sub_band_setting(AS923_sub_band) ==  0) {
        return AT_PARAM_ERROR;
    }
}
```
This changes the default frequency plan of the MAC for the Asian zone.

## 2. Channel Mask Configuration (Multichannel Subbands)
In regions like **US915** and **AU915**, subbands are not changed by altering the whole region's plan, but by applying a **channel mask (Channel Mask)**.

- **Command**: `AT+CHMASK=<M0>:<M1>:<M2>:<M3>:<M4>:<M5>`
- **Backend function**: `AT_Channel_Mask_set` in `lora_at.c` (line 2054)

### Channel Mask Structure
The `enable_chmask[6]` variable is an array of 6 unsigned 16-bit integers (`uint16_t`). 
This allows addressing a maximum of 6 x 16 = 96 channels.

- `enable_chmask[0]`: Channels 0 to 15
- `enable_chmask[1]`: Channels 16 to 31
- `enable_chmask[2]`: Channels 32 to 47
- `enable_chmask[3]`: Channels 48 to 63
- `enable_chmask[4]`: Channels 64 to 79
- `enable_chmask[5]`: Channels 80 to 95

### Special Treatment by Region (US915 / AU915 vs EU868)
When requesting or querying the Channel Mask (`AT+CHMASK=?`), the firmware checks the active region (`LmHandlerGetActiveRegion`):
```c
if( (region == LORAMAC_REGION_US915 || region == LORAMAC_REGION_AU915)) {
    mask_num = 6;
} else {
    mask_num = 1;
}
```
* **EU868 / IN865 / AS923**: They only return or usually utilize the first offset `channel_mask[0]` (16 channels).
* **US915 / AU915**: They use the full 6 blocks. The standard dictates 72 channels for these regions:
  * Channels **0 to 63**: Narrowband uplink channels (125 kHz).
  * Channels **64 to 71**: Wideband uplink channels (500 kHz).

### Potential Anomaly / Warning in Active Firmware
There is a peculiarity in how the Channel Mask is mathematically validated at line 2064:
```c
if( 1<<REGION_NVM_MAX_NB_CHANNELS%16 <= enable_chmask[5]) // REGION_NVM_MAX_NB_CHANNELS 72
{
    AT_PRINTF("Error: Unusable channel mask is set\r\n");
    return AT_PARAM_ERROR;
}
```
It validates that the `enable_chmask[5]` index (channels 80 to 95) does not exceed the bit shift `72 % 16` (which is `8`). This restricts `enable_chmask[5]` to only using bits 0 to 7 (max value 0x00FF).
However, for 72 channels, the last valid channel is **71**, which is mathematically located in **`enable_chmask[4]`** (bit 7 of offset 4). Index 5 corresponds to channels `80-95`, which shouldn't be used in US915 at all. The correct offset to validate against the 72 channel upper limit should be index `[4]`.

## 3. Subband Configuration Example (E.g. US915 / AU915 Subband 2)
To properly select **Subband 2** (channels 8 to 15 in 125kHz, and channel 65 in 500kHz), the correct encoding would be:
- **`enable_chmask[0]`**: `0xFF00` (Activates bit 8 to 15)
- **`enable_chmask[1]` to `[3]`**: `0x0000`
- **`enable_chmask[4]`**: `0x0002` (Activates bit 1 of this block, channel 65)
- **`enable_chmask[5]`**: `0x0000`

**Resulting Command**: `AT+CHMASK=FF00:0000:0000:0000:0002:0000`

## Conclusion
1. Frequencies and macro plans are altered with `AT+BAND`, supported in the case of `AS923` by a regional subband id (specific in the LoRaMac-Node layer).
2. The use of subbands for 72-channel architectures (US915 / AU915) relies strictly on modifying the `AT+CHMASK` bitmask and managing the 6 hex blocks properly, taking care of both the low and high-speed banks (500 kHz channel).
