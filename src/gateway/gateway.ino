//
// Lolcat Gateway, Lora to Serial
//

#define DEBUG true //enable Serial logging
#define SEND_TO_ESP false //enable data transmission to ESP through SerialSoftware

#include <SoftwareSerial.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// Network Adresses
#define GATEWAY_ADDRESS 254
#define CAT1 1

// PIn Configuration for LoRa Featherwing
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2


SoftwareSerial espSerial(5,6); // RX, TX for Serial transmettion to the ESP8266

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, GATEWAY_ADDRESS);

void setup() 
{
  
  setupLog();
  setupEspSerial();

  pinMode(RFM95_RST, OUTPUT);
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  log("Initialisation LoRa");
  if (!manager.init())
    log("Erreur d'initialisation");
    
  manager.setTimeout(500);
  //manager.setRetries(5);
  
  //modify ModemConfig for a Slow+long range.
  //driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
  driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  driver.setTxPower(23, false);

  sendToEsp("#GatewayUP#");
}

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
      log("Received from Cat n"+String(from)+" : " + String((char*)buf));
      //log rssi (Received Signal Strength Indication)
      //This number will range from about -15 to about -100. The larger the number (-15 being the highest you'll likely see) 
      log("RSSI: " + String(driver.lastRssi(), DEC));
      
      sendToEsp((char*)buf);
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

