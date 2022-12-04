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

// framework include
#include "Arduino.h"

// custom include
#include "sensor/sensor_DHT22.h"
// #include "wifi/wifi.h"
#include "wireless/wireless.h"

// for logging to thingspeak
#include "ThingSpeak.h"
#include "secrets.h"

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

void setup() {
  initSerial();
  initSensors();
  initWIFI();
  initFans();

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