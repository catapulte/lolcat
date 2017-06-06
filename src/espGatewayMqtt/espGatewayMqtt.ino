//
// Lolcat ESP Gateway, Serial to Mqtt
//

#define DEBUG false //enable Serial logging

#include "SoftwareSerial.h"
#include "PubSubClient.h"
#include <ESP8266WiFi.h>


#define mqtt_server "simple.lolcat.passoire.net"
#define mqtt_port 1883

#define mqtt_user "lolcat" 
#define mqtt_password "lolcat"
#define mqtt_topic "cat.data"



//Network Configuration
const char *ssid = "lolcat";
const char *password = "taclollol";


SoftwareSerial espSerial(0, 2, false, 256); //rx , tx

WiFiClient espClient;
PubSubClient client(espClient);


void setup()
{
  setupLog();

  setupEspSerial();

  setupWifi();

  client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
}

void loop()
{
  String readString = readSoftSerial();
  if (readString.length() > 0)
  {
    log("Mqtt send : " + readString);
    mqttSend(readString.c_str());
  }
  log("wait for data");
  delay(100);
}

String readSoftSerial()
{

  String readString="";
  //We wait for data on serial port
  while (espSerial.available() > 0)
  {
    char c = espSerial.read(); //gets one byte from serial buffer
    readString += c;
  }
  return readString;
}

void mqttSend(const char *msg)
{
  //while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_password)) {
      log("connected");
      client.publish(mqtt_topic, msg);
    } 
 // }
}

void setupWifi()
{
  log("Connexion à ");
  log(ssid);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    log(".");
  }
  log("Connecté au WiFi!");
  log("IP : " + WiFi.localIP());
}

void setupEspSerial()
{
  espSerial.begin(9600);
  while (!espSerial)
    ; // Wait for serial port to be available
  log("Liaison Serie ok!");
}

void setupLog()
{
  if (DEBUG)
  {
    Serial.begin(115200);
    while (!Serial)
      ; // Wait for serial port to be available
    log("Logging enabled");
  }
}

void log(String msg)
{
  if (DEBUG)
  {
    Serial.println(msg);
  }
}
