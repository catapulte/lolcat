//
// Lolcat Gateway, Lora to Serial
//

#define DEBUG true //enable Serial logging
#define SEND_TO_ESP true //enable data transmission to ESP through SerialSoftware

#include <SoftwareSerial.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// Network Adresses
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// PIn Configuration for M0 Feather
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2


SoftwareSerial espSerial(5,6); // RX, TX for Serial transmettion to the ESP8266

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  setupLog();
  setupEspSerial();
  
  log("Initialisation LoRa");
  if (!manager.init())
    log("Erreur d'initialisation");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(23, false);

  sendToEsp("#GatewayUP#");
}

uint8_t ack_data[] = "ACK";

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  
  //log("Attente de message");
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      sendToEsp((char*)buf);
      // Send a reply back to the originator client
      if (!manager.sendtoWait(ack_data, sizeof(ack_data), from))
        log("Impossible d'envoyer l'acquittement");
    }
  }
}

void setupLog() {
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial)
      ; // Wait for serial port to be available
  }
}
void setupEspSerial() {
  if (SEND_TO_ESP) {
    espSerial.begin(9600);
    while (!espSerial)
      ; // Wait for serial port to be available
    log("Envoi vers l'esp actif");
  }
}

void sendToEsp(String msg) {
  if (SEND_TO_ESP) {
    log("=> ESP : " + msg);
    espSerial.println(msg);
  }
}


void log(String msg) {
  if (DEBUG) {
    Serial.println(msg);
  }
}

