// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W

#define DEBUG true

#include <stdarg.h>
#include <Adafruit_GPS.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>


#define GPSSerial Serial1

Adafruit_GPS GPS(&GPSSerial);

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define LED 13

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup() {
	setupLog();
	setupGPS();
	setupLora();
}

void setupGPS() {
	log("initializing GPS");

	// 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
	GPS.begin(9600);
	// uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// uncomment this line to turn on only the "minimum recommended" data
	//GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
	// For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
	// the parser doesn't care about other sentences at this time
	// Set the update rate
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
	// For the parsing code to work nicely and have time to sort thru the data, and
	// print it out we don't suggest using anything higher than 1 Hz

	// Request updates on antenna status, comment out to keep quiet
	GPS.sendCommand(PGCMD_ANTENNA);

	delay(1000);

	// Ask for firmware version
	GPSSerial.println(PMTK_Q_RELEASE);
}

void setupLora() {
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);
  
	pinMode(LED, OUTPUT);

	if (!manager.init()) {
		log("init failed");
	}
	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
	// you can set transmitter powers from 5 to 23 dBm:
	driver.setTxPower(23, false);
}

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
	char c = GPS.read();
	//log(String(c));

  String dataStr;
  
	if (GPS.newNMEAreceived()) {
		//log(GPS.lastNMEA()); Dont read this stupid, the flag is set to false after use
		if (GPS.parse(GPS.lastNMEA())) {
      log(" => " + String(GPS.hour) + ":" + String(GPS.minute));
      log(" => Sat: " + String(GPS.satellites) );
			if (GPS.fix) {
        log("FIXED!");
        dataStr = "#" + String(GPS.latitude,4)
                      + String(GPS.lat)
                      + "|"
                      + String(GPS.longitude,4)
                      + String(GPS.lon)
                      +"|"
                      + String(GPS.angle)
                      +"|"
                      + String(GPS.altitude)
                      +"|"
                      + String(GPS.day, DEC)+ String('/')
                      + String(GPS.month, DEC)
                      + String("/20") + String(GPS.year, DEC)
                      + "@"
                      + String(GPS.hour, DEC) + String(':')
                      + String(GPS.minute, DEC) + String(':')
                      + String(GPS.seconds, DEC) + String('.')
                      + String(GPS.milliseconds) + "#";
        log(dataStr);
			} else {
        log("No GPS fix");
        dataStr="F*** i'm lost";
        return;
			}
		} else {
			log("unable to parse NMEA sentence");
      return;
    }
	} else {
		//log("no NMEA sentence received");
    return;
	}
 
	// Send a message to manager_server
	if (manager.sendtoWait((uint8_t*)dataStr.c_str(), dataStr.length(), SERVER_ADDRESS)) {
		// Now wait for a reply from the server
		uint8_t len = sizeof(buf);
		uint8_t from;
		if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
			log("Message ACK");
		} else {
			log("No reply, is rf95_reliable_datagram_server running?");
			digitalWrite(LED, HIGH);
		}
	} else {
		log("sendtoWait failed");
    //TODO if Failed, add in FIFO (with limited size) to keep data & send when link with gateway is back.
		digitalWrite(LED, HIGH);
	}
	delay(5000);
	digitalWrite(LED, LOW);
	delay(200);
	digitalWrite(LED, HIGH);
	delay(200);
	digitalWrite(LED, LOW);
}

void setupLog() {
	if (DEBUG) {
		Serial.begin(19200);
		while (!Serial)
			; // Wait for serial port to be available
	}
}

void logf(char *fmt, ...) {
	if (DEBUG) {
		char buf[128]; // resulting string limited to 128 chars
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, 128, fmt, args);
		va_end(args);
		Serial.println(buf);
	}
}

void log(String msg) {
	if (DEBUG) {
		Serial.println(msg);
	}
}

