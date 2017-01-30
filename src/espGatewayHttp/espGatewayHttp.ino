//
// Lolcat ESP Gateway, Serial to Wifi
//

#define DEBUG true //enable Serial logging

#include "SoftwareSerial.h"
#include <ESP8266WiFi.h>

//Http Server
#define http_server "192.168.1.16"
#define http_port 8080
#define endPoint "/location?rawdata="


//Network Configuration
const char *ssid = "Livebox-07F0";
const char *password = "AurelieYoan83";

SoftwareSerial espSerial(0, 2, false, 256); //rx , tx

void setup()
{
  setupLog();

  setupEspSerial();

  setupWifi();
}

void loop()
{

  String readString = readSoftSerial();

  if (readString.length() > 0)
  {
    log("http send : " + readString);
    httpSend(readString.c_str());
  }
  log("wait for data");
  delay(100);
}

String readSoftSerial()
{

  String readString;
  //We wait for data on serial port
  while (espSerial.available() > 0)
  {
    char c = espSerial.read(); //gets one byte from serial buffer
    readString += c;
    delay(10);
  }
  return readString;
}

void httpSend(const char *msg)
{
  WiFiClient client;
  if (!client.connect(http_server, http_port))
  {
    log("Erreur de connexion");
    return;
  }

  String url = endPoint + String(msg);
  log("Requesting URL: " + url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + http_server + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      log(">>> Client Timeout !");
      client.stop();
      return;
    }
    // Read all the lines of the reply from server and print them to Serial
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      log(line);
    }
  }
  log("closing connection");
}

void setupWifi()
{
  log("Connexion à ");
  log(ssid);

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
  }
}

void log(String msg)
{
  if (DEBUG)
  {
    Serial.println(msg);
  }
}
