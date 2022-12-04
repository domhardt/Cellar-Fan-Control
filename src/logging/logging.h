#pragma once

// framework include
#include "Arduino.h"
#include "../wireless/wireless.h"
#include "../sensor/sensor_DHT22.h"

void initSerial ();
void printValuesToSerial(String location, float humidity, float temperature, float dewPoint);
void printDebugInformation();