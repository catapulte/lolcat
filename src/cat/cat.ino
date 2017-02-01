//
// Lolcat transmitter
//

#define DEBUG true
#define GATEWAY_COMM_TIME_INTERVAL 5000


#include <stdarg.h>
#include <Adafruit_GPS.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 11   

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT_Unified dht(DHTPIN, DHTTYPE);

#define GPSSerial Serial1

Adafruit_GPS GPS(&GPSSerial);

#define CAT1 1

#define GATEWAY_ADDRESS 254


#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define VBATPIN A7

#define LED 13

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CAT1);


void setup() {
	setupLog();
	setupGPS();
	setupLora();
  dht.begin();
}

void setupGPS() {
	log("initializing GPS");

	// 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
	GPS.begin(9600);
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
	GPS.sendCommand(PGCMD_ANTENNA);

	delay(1000);

	// Ask for firmware version
	GPSSerial.println(PMTK_Q_RELEASE);
}

void setupLora() {
  
	pinMode(RFM95_RST, OUTPUT);
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
	pinMode(LED, OUTPUT);

	if (!manager.init()) 
		log("Erreur d'initialisation");
  
  manager.setTimeout(500);
  //manager.setRetries(5);
  
  //modify ModemConfig for a Slow+long range.
  //driver.setModemConfig(RH_RF95::Bw31_25Cr48Sf512);
  driver.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
	driver.setTxPower(23, false);

  sendData("#"+String(CAT1)+"|initDone#");
  return;
}

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {

  //Retrieve data from GPS  
  //ex #1|4810.8477N|126.8529W|346.34|107.00|6|29/1/2017@21:11:42.0|26.50|29.80|4.07#
  String gpsData="";//getGPSData();

  //no GPS data => exit from the loop and try again
  if (strstr(gpsData.c_str(), "ERROR") != NULL) {
    log(gpsData);
    return;
  }

  String dhtData="";//getDHTData();
  log(dhtData);

  //format data
  /*String dataStr = "#" + String(CAT1);
                      + "|"
                      + gpsData
                      + "|" 
                      + dhtData
                      + "|" 
                      + String(getBatteryStatus())+ "#";*/
                      
  // String dataStr = "Hello world";
  String dataStr = "1 4810.8477N 126.8529W 346.34 107.00 6 29/1/2017@21:11:42.0 26.50 29.80 4.07";

	//send to lora network
	sendData(dataStr);
  
	delay(GATEWAY_COMM_TIME_INTERVAL);

	digitalWrite(LED, LOW);
	delay(200);
	digitalWrite(LED, HIGH);
	delay(200);
	digitalWrite(LED, LOW);
}


String getGPSData(){
  String dataStr;
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (GPS.parse(GPS.lastNMEA())) {
      log(" => " + String(GPS.hour) + ":" + String(GPS.minute));
      log(" => Sat: " + String(GPS.satellites) );
      if (GPS.fix) {
        dataStr = String(GPS.latitude,4)
                      + String(GPS.lat)
                      + "|"
                      + String(GPS.longitude,4)
                      + String(GPS.lon)
                      +"|"
                      + String(GPS.angle)
                      +"|"
                      + String(GPS.altitude)
                      +"|"
                      + String(GPS.satellites)
                      +"|"
                      + String(GPS.day, DEC)+ String('/')
                      + String(GPS.month, DEC)
                      + String("/20") + String(GPS.year, DEC)
                      + "@"
                      + String(GPS.hour, DEC) + String(':')
                      + String(GPS.minute, DEC) + String(':')
                      + String(GPS.seconds, DEC) + String('.')
                      + String(GPS.milliseconds);
      } else {
        dataStr="ERROR:NoGPS";
      }
    } else {
      dataStr="ERROR:ParseError";
    }
  } else {
    dataStr="ERROR:NoNMEA";
  }
  return dataStr;
}
String getDHTData(){

  sensors_event_t eventtemp;  
  dht.temperature().getEvent(&eventtemp);
  
  sensors_event_t eventhum;  
  dht.humidity().getEvent(&eventhum);
  
  float h = eventhum.relative_humidity;
  float t = eventtemp.temperature;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    log("Failed to read from DHT sensor!");
    return "0|0";
  }
  return String(t,2)+"|"+String(h,2);
}

boolean sendData(String dataStr) {
  log("Sending : "+dataStr);
  log("Size : "+String(dataStr.length()));
  
  // Send a message to gateway
  if (!manager.sendtoWait((uint8_t*)dataStr.c_str(), dataStr.length(), GATEWAY_ADDRESS)) {
  //if (!manager.sendtoWait((uint8_t*)dataStr.c_str(), dataStr.length(), RH_BROADCAST_ADDRESS)) {
    log("sendtoWait failed");
    //TODO if Failed, add in FIFO (with limited size) to keep data & send when link with gateway is back.
    digitalWrite(LED, HIGH);
  }
  
  //put lora radio into low-power sleep mode
  driver.sleep();
}

float getBatteryStatus() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
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

