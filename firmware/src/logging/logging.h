#pragma once

// framework include
#include "Arduino.h"

// custom include
#include "../wireless/wireless.h"
#include "../sensor/sensor_BME280.h"
#include "../finiteStateMachine/finiteStateMachine.h"

// library include
#include "ThingSpeak.h"
#include "secrets.h"

void initSerial ();
void initThingSpeak ();
void printValuesToSerial(String location, float humidity, float temperature, float dewPoint);
void printDebugInformation();
void logToThingSpeak();

String fanInfo ();
String humidityInfo ();
String temperatureInfo ();
String dewPointInfo ();