# Análisis del Channel Mask y Bandas en el Firmware del LSM1x0A

Basado en el código fuente de la aplicación principal (`LSM1x0A_FW/LoRaWAN/App`) y el hardware, aquí se detalla cómo el firmware del módulo procesa y maneja las bandas (Regiones), subbandas y el _Channel Mask_ según lo implementado a través de su interfaz de comandos AT.

El _MiddleWare_ (Stack LoRaWAN y definiciones internas de LoraMac-Node como `RegionAS923.h` o `LmHandler`) está precompilado o fuera del directorio del proyecto, pero la interacción directa con él se realiza en `lora_at.c` y `lora_command.c`.

## 1. Configuración de Bandas (Regiones)
La frecuencia/banda principal se manipula con el comando **`AT+BAND`**.

- **Comando**: `AT+BAND=<BandID>[,<SubBand>]`
- **Valores posibles (`BandID`)**:
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

### Subbandas Fijas por Región (Caso AS923)
La función `AT_Region_set` en `lora_at.c` (línea 1306) espera hasta dos parámetros separados por coma. Específicamente, para la región **AS923**, se requiere indicar una **subbanda** explícitamente:
```c
parameter_num = tiny_sscanf(param, "%hhu,%hhu", &region, &AS923_sub_band);
if(region == LORAMAC_REGION_AS923) {
    if(AS923_sub_band_setting(AS923_sub_band) ==  0) {
        return AT_PARAM_ERROR;
    }
}
```
Esto cambia el plan de frecuencias predeterminado del MAC para la zona asiática.

## 2. Configuración del Channel Mask (Subbandas Multicanal)
En regiones como **US915** y **AU915**, las subbandas no se cambian alterando el plan de la región completa, sino aplicando una **máscara de canales (Channel Mask)**.

- **Comando**: `AT+CHMASK=<M0>:<M1>:<M2>:<M3>:<M4>:<M5>`
- **Función backend**: `AT_Channel_Mask_set` en `lora_at.c` (línea 2054)

### Estructura del Channel Mask
La variable `enable_chmask[6]` es un array de 6 enteros de 16 bits sin signo (`uint16_t`). 
Esto permite direccionar un máximo de $6 \times 16 = 96$ canales.

- `enable_chmask[0]`: Canales 0 al 15
- `enable_chmask[1]`: Canales 16 al 31
- `enable_chmask[2]`: Canales 32 al 47
- `enable_chmask[3]`: Canales 48 al 63
- `enable_chmask[4]`: Canales 64 al 79
- `enable_chmask[5]`: Canales 80 al 95

### Tratamiento Especial por Region (US915 / AU915 vs EU868)
Al solicitar u obtener el Channel Mask (`AT+CHMASK=?`), el firmware revisa cuál es la región activa (`LmHandlerGetActiveRegion`):
```c
if( (region == LORAMAC_REGION_US915 || region == LORAMAC_REGION_AU915)) {
    mask_num = 6;
} else {
    mask_num = 1;
}
```
* **EU868 / IN865 / AS923**: Solamente devuelven o suelen utilizar el primer offset `channel_mask[0]` (16 canales).
* **US915 / AU915**: Usan los 6 bloques completos. El estándar dicta 72 canales para estas regiones:
  * Canales **0 a 63**: Canales de subida de banda estrecha (125 kHz).
  * Canales **64 a 71**: Canales de subida de banda ancha (500 kHz).

### Posible Anomalía / Advertencia en el Firmware Activo
Existe una peculiaridad en cómo se valida matemáticamente el Channel Mask en la línea 2064:
```c
if( 1<<REGION_NVM_MAX_NB_CHANNELS%16 <= enable_chmask[5]) // REGION_NVM_MAX_NB_CHANNELS 72
{
    AT_PRINTF("Error: Unusable channel mask is set\r\n");
    return AT_PARAM_ERROR;
}
```
Se valida que el índice `enable_chmask[5]` (canales 80 a 95) no supere el corrimiento del bit `72 % 16` (que es `8`). Esto restringe `enable_chmask[5]` a usar solo los bits del 0 al 7 (valor max 0x00FF).
No obstante, para 72 canales, el último canal válido es el **71**, el cual se encuentra matemáticamente en **`enable_chmask[4]`** (el bit 7 del offset 4). El índice 5 corresponde a los canales `80-95`, los cuales no deberían usarse en US915 en absoluto. El offset correcto a validar contra el límite superior de los 72 canales debería ser el índice `[4]`.

## 3. Ejemplo de Configuración de Subbanda (Ej. US915 / AU915 Subbanda 2)
Para seleccionar correctamente la **Subbanda 2** (canales 8 al 15 en 125kHz, y el canal 65 en 500kHz), la codificación correcta sería:
- **`enable_chmask[0]`**: `0xFF00` (Activa el bit 8 al 15)
- **`enable_chmask[1]` a `[3]`**: `0x0000`
- **`enable_chmask[4]`**: `0x0002` (Activa el bit 1 de este bloque, canal 65)
- **`enable_chmask[5]`**: `0x0000`

**Comando Resultante**: `AT+CHMASK=FF00:0000:0000:0000:0002:0000`

## Conclusión
1. Las frecuencias y planes macro se alteran con `AT+BAND`, apoyadas en caso de `AS923` por un id de subbanda regional (específico en la capa LoRaMac-Node).
2. El uso de subbandas para arquitecturas de 72 canales (US915 / AU915) recae estrictamente en modificar el bitmask `AT+CHMASK` y gestionar los 6 bloques hexadecimales adecuadamente, cuidando tanto del banco de baja como de alta velocidad (canal de 500 kHz).
