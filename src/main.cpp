/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include "Arduino.h"

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// needed for WIFI Manager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// needed for HTTP requests
#include <ESP8266HTTPClient.h>

const String FAN_REQUEST_OFF = "/cm?cmnd=Power%20Off";
const String FAN_REQUEST_ON = "/cm?cmnd=Power%20On";
const String FAN_REQUEST_STATUS = "/cm?cmnd=Power";

HTTPClient sender;
WiFiClient wifiClient;

boolean fanStatusWorkshop;
boolean fanStatusPantry;

// needed for DTH sensors
#include "DHT.h"
#define DHT_TYPE DHT22
 
const int DHT_INSIDE_PIN = 5;
const int DHT_OUTSIDE_PIN = 12;
const int DHT_POWER_PIN = 2;
const float DEW_POINT_THRESHOLD = 5.0;// in °C

DHT dhtInside(DHT_INSIDE_PIN, DHT_TYPE);
DHT dhtOutside(DHT_OUTSIDE_PIN, DHT_TYPE);

float humidityInside;
float temperatureInside;
float dewPointInside;

float humidityOutside;
float temperatureOutside;
float dewPointOutside;
float dewPointOutsideLastSwicthingTime;

// for serial
void initSerial () {
  Serial.begin(74880);
  // yield();
  delay(500);

  while (!Serial);
  delay(500);
  
  Serial.println();
  Serial.println("\nNAME: Humidity Controlled Cellar Fan");
  Serial.println("FILE: " __FILE__);
  Serial.println("VERSION: " __DATE__ ", " __TIME__);
  Serial.println("AUTHOR: Michael Domhardt\n");
}

void initWIFI () {
    WiFiManager wifiManager;//Local intialization. Once its business is done, there is no need to keep it around
    //wifiManager.resetSettings();//reset saved settings
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); //set custom ip for portal

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    wifiManager.autoConnect("AutoConnectAP");// and goes into a blocking loop awaiting configuration
    //wifiManager.autoConnect();    //or use this for auto generated name ESP + ChipID

    Serial.println("WIFI connected.\n)");    //if you get here you have connected to the WiFi
}

void readSensors () {
  digitalWrite(DHT_POWER_PIN, HIGH);
  delay(25);

  humidityInside = -100.0;
  temperatureInside = -100.0;

  humidityOutside = -100.0;
  temperatureOutside = -100.0;

  dhtInside.begin();
  dhtOutside.begin();
  delay(25);

  humidityInside = dhtInside.readHumidity();
  temperatureInside = dhtInside.readTemperature();

  humidityOutside = dhtOutside.readHumidity();
  temperatureOutside = dhtOutside.readTemperature();
  
  digitalWrite(DHT_POWER_PIN, LOW);
}

float calcDewPoint(float humidity, float temperature) {
  float k;
  k = log(humidity/100) + (17.62 * temperature) / (243.12 + temperature);
  return 243.12 * k / (17.62 - k);
}

void printValuesToSerial (String location, float humidity, float temperature, float dewPoint) {
  Serial.print(location + String(": "));
  Serial.print(String("temperature = ") + temperature + String("°C, "));
  Serial.print(String("humidity = ") + humidity + String("%, "));
  Serial.print(String("dewPoint = ") + dewPoint + String("°C"));
  Serial.println();
}

int fanPowerStatus (String fanName, String request) { 
  int result = -1;

  String httpRequest = "http://" + fanName + request;
  if (sender.begin(wifiClient, httpRequest)) {// establish connection, initialise request
    int httpCode = sender.GET();// HTTP-Code of the response

    if (httpCode > 0) {// request sent and server responded
      if (httpCode == HTTP_CODE_OK) { // response was OK aka code 200
        String payload = sender.getString();// response string
        Serial.println(fanName + String("PowerStatus = ") + payload); 
        
        if (payload.indexOf("ON") > 0) {
          result = 1;
        }
        else if (payload.indexOf("OFF") > 0) {
          result = 0;
        }
        else {
          Serial.println("WARNING: fan power status could not be determind.");
        }
      } 
    }
    else {// HTTP error handling
      Serial.println(String("HTTP-Error: ") + sender.errorToString(httpCode).c_str());
    }

    sender.end();// end request and terminate conection
    
  }else {
    Serial.printf("WARNING: HTTP connection could not be established.");
  }

  return result;
}



void calcDewPoints () {
  Serial.println("calcDewPoints called");
  
  readSensors ();

  dewPointInside = -100.0;
  dewPointOutside = -100.0;

  dewPointOutside = calcDewPoint(humidityOutside, temperatureOutside);
  dewPointInside = calcDewPoint(humidityInside, temperatureInside);
}

void printDebugInformation () {
  Serial.println(String("fanStatusWorkshop = ") + fanStatusWorkshop + String(", fanStatusPantry = ") + fanStatusPantry); 
  printValuesToSerial("INDOOR ", humidityInside, temperatureInside, dewPointInside);
  printValuesToSerial("OUTDOOR", humidityOutside, temperatureOutside, dewPointOutside);
}

void fansOn () {
  Serial.println("fansON called");

  dewPointOutsideLastSwicthingTime = dewPointOutside;
  fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_ON);
  fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_ON);
}

void fansOff () {
  Serial.println("fansOff called");
 
  fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_OFF);
  fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_OFF);
}

void fanControl () {
  Serial.println("fanControl called");

  boolean fanStatus = fanStatusWorkshop || fanStatusPantry;

  if (!fanStatus) {
    if ((dewPointInside - dewPointOutside > DEW_POINT_THRESHOLD) && (temperatureInside > DEW_POINT_THRESHOLD*2)) {
      fansOn();
    }
  }

  if (fanStatus) {
    // rain shutoff
    if (dewPointOutside > dewPointOutsideLastSwicthingTime + DEW_POINT_THRESHOLD) {
      fansOff();
      }
    // temperature shutoff
    if (temperatureInside < DEW_POINT_THRESHOLD*2){
     fansOff();
    }
    
  }

  printDebugInformation();
}

void setup() {
  initSerial();

  pinMode(DHT_POWER_PIN, OUTPUT);
  digitalWrite(DHT_POWER_PIN, LOW);

  initWIFI();

  fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_OFF);
  fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_OFF);

}

void loop() {

}
