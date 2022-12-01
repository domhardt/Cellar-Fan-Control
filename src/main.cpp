/*
 * Cellar Fan Control
 *
 * implementation for ESP8266
 * uses two DHT22 temperature and humidity sensors
 * controls two WIFI sockets running TASMOTA firmware (e.g., Sonoff S2x)
 * logs data to thingspeak
 * logic based on finite state machine (FSM)
 * 
 * by Michael Domhardt
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// for WIFI Manager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

// for logging to thingspeak
#include "secrets.h"
#include "ThingSpeak.h"

// for HTTP requests
#include <ESP8266HTTPClient.h>

const String FAN_REQUEST_OFF = "/cm?cmnd=Power%20Off";
const String FAN_REQUEST_ON = "/cm?cmnd=Power%20On";
const String FAN_REQUEST_STATUS = "/cm?cmnd=Power";

HTTPClient sender;
WiFiClient wifiClient;

boolean fanStatusWorkshop;
boolean fanStatusPantry;

// for DTH sensors
#include "DHT.h"
#define DHT_TYPE DHT22
 
const int DHT_INSIDE_PIN = 5;
const int DHT_OUTSIDE_PIN = 12;
const int DHT_POWER_PIN = 2;

const float DEW_POINT_THRESHOLD = 4.0;// in K
const float MIN_INSIDE_TEMPERATURE_CUTOFF = 8.0;// in °C
const float MIN_OUTSIDE_TEMPERATURE_CUTOFF = -10.0;// in °C

const unsigned long MEASURE_INTERVAL = 3 * 60 * 1000; // in millis, default: 3 min 

unsigned long measurementTimestamp = 0;

DHT dhtInside(DHT_INSIDE_PIN, DHT_TYPE);
DHT dhtOutside(DHT_OUTSIDE_PIN, DHT_TYPE);

float humidityInside;
float temperatureInside;
float dewPointInside;

float humidityOutside;
float temperatureOutside;
float dewPointOutside;
float dewPointInsideLastSwicthingTime;

// for state machine
const unsigned long VENTILATION_INTERVAL = 30 * 60 * 1000; // in millis, default: 30 min 
const unsigned long WAIT_INTERVAL = 90 * 60 * 1000; // in millis, default: 90 min

unsigned long stateStartTimeStamp = 0;

enum states {
  NONE,
  MEASURING,
  VENTILATING,
  WAITING
};

states state, priorState;
const char* stateStr[] = {"NONE", "MEASURING", "VENTILATING", "WAITING"};

// helper methods

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

void calcDewPoints () {
  dewPointInside = -100.0;
  dewPointOutside = -100.0;

  dewPointOutside = calcDewPoint(humidityOutside, temperatureOutside);
  dewPointInside = calcDewPoint(humidityInside, temperatureInside);

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
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

void fansOn () {
  fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_ON);
  fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_ON);

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
}

void fansOff () { 
  fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_OFF);
  fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_OFF);

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
}

void logToThingSpeak () {
  ThingSpeak.setStatus(String("State: ") + stateStr[state]);

  ThingSpeak.setField(1, fanStatusWorkshop);
  ThingSpeak.setField(2, fanStatusPantry);
  ThingSpeak.setField(3, humidityInside);
  ThingSpeak.setField(4, humidityOutside);
  ThingSpeak.setField(5, temperatureInside);
  ThingSpeak.setField(6, temperatureOutside);
  ThingSpeak.setField(7, dewPointInside);
  ThingSpeak.setField(8, dewPointOutside);
  
  int statusMessage = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);

  if(statusMessage == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(statusMessage));
  }

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
}

void printValuesToSerial (String location, float humidity, float temperature, float dewPoint) {
  Serial.print(location + String(": "));
  Serial.print(String("temperature = ") + temperature + String("°C, "));
  Serial.print(String("humidity = ") + humidity + String("%, "));
  Serial.print(String("dewPoint = ") + dewPoint + String("°C"));
  Serial.println();
}

void printDebugInformation () {
  Serial.println(String("fanStatusWorkshop = ") + fanStatusWorkshop + String(", fanStatusPantry = ") + fanStatusPantry); 
  printValuesToSerial("INDOOR ", humidityInside, temperatureInside, dewPointInside);
  printValuesToSerial("OUTDOOR", humidityOutside, temperatureOutside, dewPointOutside);
}

void takeMeasurements () {
 boolean measurementIntervalExpired = millis() > measurementTimestamp + MEASURE_INTERVAL;

  if (measurementIntervalExpired) {
    measurementTimestamp =  millis();
    readSensors();
    calcDewPoints();
    logToThingSpeak();
    printDebugInformation();
  }

}

void measure () {
  // initialise stuff on entering a state
  if (state != priorState) { 
    Serial.println(String("entering state ") + __PRETTY_FUNCTION__);
    priorState = state;
  }

  // do state stuff repeatedly
  takeMeasurements();

  // check for state transition
  boolean humidityDifferenceHigh = dewPointInside - dewPointOutside > DEW_POINT_THRESHOLD;
  boolean temperatureInsideTooLow = temperatureInside < MIN_INSIDE_TEMPERATURE_CUTOFF;
  boolean temperatureOutsideTooLow = temperatureOutside < MIN_OUTSIDE_TEMPERATURE_CUTOFF;

  boolean leaveStateToVentilate = humidityDifferenceHigh && !temperatureInsideTooLow &&  !temperatureOutsideTooLow;
   
  if (leaveStateToVentilate) {
    state = VENTILATING;
  }

  // clean up on leaving a state
  if (state != priorState) {  
    Serial.println(String("leaving state ") + __PRETTY_FUNCTION__ );
  }
}

void ventilate () {
  if (state != priorState) { // initialise stuff on entering a state
    Serial.println(String("entering state ") + __PRETTY_FUNCTION__ );
    priorState = state;
    stateStartTimeStamp = millis();
    dewPointInsideLastSwicthingTime = dewPointInside;
    fansOn();
  }

  // do state stuff repeatedly
  takeMeasurements();

  // check for state transition
  boolean ventilationIntervalExpired = millis() > stateStartTimeStamp + VENTILATION_INTERVAL;
  boolean temperatureInsideTooLow = temperatureInside < MIN_INSIDE_TEMPERATURE_CUTOFF;
  boolean temperatureOutsideTooLow = temperatureOutside < MIN_OUTSIDE_TEMPERATURE_CUTOFF;
  boolean humidityOutsideTooHigh = dewPointOutside > dewPointInsideLastSwicthingTime - (DEW_POINT_THRESHOLD/2);

  boolean leaveStateToWaiting = ventilationIntervalExpired || temperatureInsideTooLow || temperatureOutsideTooLow || humidityOutsideTooHigh;

  if (leaveStateToWaiting) {
    Serial.println(String("ventilationIntervalExpired=") + ventilationIntervalExpired);
    Serial.println(String("temperatureInsideTooLow=") + temperatureInsideTooLow);
    Serial.println(String("humidityOutsideTooHigh=") + humidityOutsideTooHigh);
    state = WAITING;
  }
  
  // clean up on leaving a state
  if (state != priorState) {  
    fansOff();
    Serial.println(String("leaving state ") + __PRETTY_FUNCTION__);
  }
}

void wait () {
  if (state != priorState) { // initialise stuff on entering a state
    Serial.println(String("entering state ") + __PRETTY_FUNCTION__ );
    priorState = state;
    stateStartTimeStamp = millis();
  }

  // do state stuff repeatedly
  takeMeasurements();

  // check for state transition
  boolean waitIntervalExpired = millis() > stateStartTimeStamp + WAIT_INTERVAL;
 
  boolean leaveStateToMeasuring = waitIntervalExpired;

  if (leaveStateToMeasuring) {
    state = MEASURING;
  }

  // clean up on leaving a state
  if (state != priorState) {
    Serial.println(String("leaving state ") + __PRETTY_FUNCTION__ );
  }
}

// for serial
void initSerial () {
  Serial.begin(74480);
  delay(500);

  while (!Serial);
  delay(500);
  
  Serial.println();
  Serial.println("\nNAME: Humidity Controlled Cellar Fan");
  Serial.println("FILE: " __FILE__);
  Serial.println("VERSION: " __DATE__ ", " __TIME__);
  Serial.println("AUTHOR: Michael Domhardt\n");

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
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
    Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
}

void setup() {
  initSerial();

  pinMode(DHT_POWER_PIN, OUTPUT);
  digitalWrite(DHT_POWER_PIN, LOW);

  initWIFI();
  fansOff();

  ThingSpeak.begin(wifiClient);  // Initialize ThingSpeak

  priorState = NONE;
  state = MEASURING;

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__ );
}

void loop() {
  // run finite state machine
  switch (state) {  
    case NONE:
      state = MEASURING;
      break;
    case MEASURING:
      measure();
      break;
    case VENTILATING:
      ventilate();
      break;
    case WAITING:
      wait();
      break;
  }
}