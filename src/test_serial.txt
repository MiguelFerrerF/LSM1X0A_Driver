#include "SerialDriver.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <cstdio>
#include <string.h>

// Definición de pines para el ESP32 (ajustar según el diseño)
#define UART_TX_PIN 33
#define UART_RX_PIN 14
#define BAUD_RATE 9600

HardwareSerial Serial_2(2); // Usamos UART2 para la comunicación con el módem
// Instancia del driver de comunicación
SerialDriver g_serial_driver(Serial_2);

// La función de callback que pertenece a la Capa de Protocolo (ATParser)
void handle_modem_response(const uint8_t *data, size_t len) {
  // Esta función sería el punto de entrada para el ATParser.cpp
  printf("Capa Protocolo (ATParser): %s", (const char *)data);

  // Aquí se implementaría la lógica para parsear la respuesta (OK, ERROR,
  // +EVT:...)

  // Ejemplo de parsing directo:
  if (strstr((const char *)data, "OK")) {
    printf(" -> Respuesta OK detectada.\n");
  }
}

void sendCommand(const char *cmd) {
  printf("Enviando comando: %s", cmd);
  g_serial_driver.write(reinterpret_cast<const uint8_t *>(cmd), strlen(cmd));
  vTaskDelay(pdMS_TO_TICKS(2000));
}

void executeAllCommands() {
  const char *commands[] = {
      "AT?\r\n",
      "AT+<CMD>?\r\n",
      "AT+<CMD>\r\n",
      "AT+<CMD>=<value>\r\n",
      "AT+<CMD>=?\r\n",
      "ATZ\r\n",
      "AT+RFS\r\n",
      "AT+VL=<Level>\r\n",
      "AT+LTIME\r\n",
      "AT+APPEUI=<XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+NWKKEY=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+APPKEY=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+NWKSKEY=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+APPSKEY=<XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+DADDR=<XXXXXXXX>\r\n",
      "AT+DEUI=<XX:XX:XX:XX:XX:XX:XX:XX>\r\n",
      "AT+NWKID=<NwkID>\r\n",
      "AT+JOIN=<Mode>\r\n",
      "AT+LINKC\r\n",
      "AT+SEND=<Port>:<Ack>:<Payload>\r\n",
      "AT+VER\r\n",
      "AT+ADR=<ADR>\r\n",
      "AT+DR=<DataRate>\r\n",
      "AT+BAND=<BandID>\r\n",
      "AT+CLASS=<Class>\r\n",
      "AT+DCS=<DutyCycle>\r\n",
      "AT+JN1DL=<Delay>\r\n",
      "AT+JN2DL=<Delay>\r\n",
      "AT+RX1DL=<Delay>\r\n",
      "AT+RX2DL=<Delay>\r\n",
      "AT+RX2DR=<DataRate>\r\n",
      "AT+RX2FQ=<Freq>\r\n",
      "AT+TXP=<Power>\r\n",
      "AT+TTONE\r\n",
      "AT+TRSSI\r\n",
      "AT+TCONF=<Freq in Hz>:<Power in dBm>:<Lora Bandwidth <0 to 6>, or FSK "
      "Bandwidth in Hz>:<Lora SF or FSK datarate (bps)>:<CodingRate 4/5, 4/6, "
      "4/7, 4/8>:<Lna>:<PA Boost>:<Modulation 0:FSK, 1:Lora, "
      "2:BPSK>:<PayloadLen in Bytes>:<FskDeviation in Hz>:<LowDrOpt 0:off, "
      "1:on, 2:Auto>:<BTproduct: 0:no Gaussian Filter Applied, 1:BT=0,3, "
      "2:BT=0,5, 3:BT=0,7, 4:BT=1>\r\n",
      "AT+TTX=<PacketNb>\r\n",
      "AT+TRX=<PacketNb>\r\n",
      "AT+TTH=<Fstart>,<Fstop>,<Fdelta>,<PacketNb>\r\n",
      "AT+TOFF\r\n",
      "AT+BAT\r\n",
      "AT+MODE\r\n",
      "AT+MTX\r\n",
      "AT+MRX\r\n",
      "AT+PCONF=<Freq in Hz>:<Power in dBm>:<Lora Bandwidth <0 to 6>, or Rx "
      "FSK Bandwidth in Hz>:<Lora SF or FSK datarate (bps)>:<CodingRate 4/5, "
      "4/6, 4/7, 4/8>:<Lna>:<PA Boost>:<Modulation 0:FSK, 1:Lora, "
      "2:BPSK>:<PayloadLen in Bytes>:<FskDeviation in Hz>:<LowDrOpt 0:off, "
      "1:on, 2:Auto>:<BTproduct: 0:no Gaussian Filter Applied, 1:BT=0,3, "
      "2:BT=0,5, 3:BT=0,7, 4:BT=1>\r\n",
      "AT+PCONF=868000000:14:4:12:4/5:0:0:1:16:25000:2:3\r\n",
      "AT+PSEND=<Payload>\r\n",
      "AT+PRECV=<Timeout>\r\n",
      "AT$SSWVER\r\n",
      "AT+NWKTYPE=<NetworkType>\r\n",
      "AT+ABPFCNT=?\r\n"};

  size_t numCommands = sizeof(commands) / sizeof(commands[0]);

  for (size_t i = 0; i < numCommands; ++i) {
    sendCommand(commands[i]);
  }
}

void custom_main(void) {
  printf("--- Inicializando Capa de Comunicación ---\n");

  // 1. Inicializar el driver de UART
  if (g_serial_driver.init(BAUD_RATE, UART_RX_PIN, UART_TX_PIN,
                           handle_modem_response)) {
    printf("UART Driver iniciado con éxito.\n");
  } else {
    printf("FALLO al iniciar el UART Driver.\n");
    return;
  }

  // --- Simulación de Comandos AT ---
  sendCommand("ATZ\r\n");       // Comando AT básico
  sendCommand("AT+MODE=1\r\n"); // Set modo LoRaWAN
  sendCommand("AT?\r\n");       // Comando AT de consulta
}

void setup() {
  // setup serial port to 115200 baud
  Serial.begin(115200);
  custom_main();
}

void loop() {
  // Aquí puedes agregar lógica adicional si es necesario
}