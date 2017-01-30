//
// Lolcat transmitter
//

#define DEBUG true
#define GATEWAY_COMM_TIME_INTERVAL 5000


#include <stdarg.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CAT1 1
#define GATEWAY_ADDRESS 254

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define LED 13

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CAT1);


void setup() {
	setupLog();
	setupLora();
}

void setupLora() {

	pinMode(LED, OUTPUT);

	if (!manager.init()) 
		log("Erreur d'initialisation");
   driver.setTxPower(24, false);
}

void loop() {

	//send to lora network
	sendData();
  
	delay(GATEWAY_COMM_TIME_INTERVAL);

	digitalWrite(LED, LOW);
	delay(200);
	digitalWrite(LED, HIGH);
	delay(200);
	digitalWrite(LED, LOW);
}

boolean sendData() {
  log("Sending: #1|4810.8477N|126.8529W|346.34|107.00|6|29/1/2017@21:11:42.0|26.50|29.80|4.07#");
  uint8_t data[] = "#1|4810.8477N|126.8529W|346.34|107.00|6|29/1/2017@21:11:42.0|26.50|29.80|4.07#";
  // Send a message to gateway
  if (!manager.sendtoWait(data, sizeof(data), GATEWAY_ADDRESS)) {
    log("sendtoWait failed");
    //TODO if Failed, add in FIFO (with limited size) to keep data & send when link with gateway is back.
    digitalWrite(LED, HIGH);
  }
}

void setupLog() {
	if (DEBUG) {
		Serial.begin(19200);
		while (!Serial)
			; // Wait for serial port to be available
	}
}

void log(String msg) {
	if (DEBUG) {
		Serial.println(msg);
	}
}

