//
// Lolcat ESP Gateway, Serial to Wifi
//

#define DEBUG true //enable Serial logging

#include "SoftwareSerial.h"
#include <ESP8266WiFi.h>

//Network Configuration
const char *ssid = "catap";
const char *password = "miaoumiaou";
//Api host
const char *host = "192.168.10.114";
const int httpPort = 8080;

SoftwareSerial espSerial(0, 2, false, 256); //rx , tx

void setup()
{

  setupLog();

  setupEspSerial();

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

void loop()
{

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  // We now create a URI for the request
  String url = "/location/?rawdata=";
  String readString;

  //We wait for data on serial port
  while (espSerial.available() > 0)
  {
    char c = espSerial.read(); //gets one byte from serial buffer
    readString += c;
  }
  log("=> ARDUINO : " + readString);

  if (readString.length() > 0)
  {
    log(readString); //see what was received

    if (!client.connect(host, httpPort))
    {
      log("Erreur de connexion");
      return;
    }

    url += readString;
    log("Requesting URL: " + url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
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
  log("Heartbeat debug");
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
