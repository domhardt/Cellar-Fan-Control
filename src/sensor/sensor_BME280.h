#pragma once

// for sensors
#define BME280_POWER_PIN 15

#define BME280_ADDRESS_INSIDE 0x76
#define BME280_ADDRESS_OUTSIDE 0x77

extern unsigned long measureInterval;
extern unsigned long measurementTimestamp;

extern float dewPointThreshold;
extern float minInsideTemperatureCutoff;
extern float minOutsideTemperatureCutoff;

extern float humidityInside;
extern float temperatureInside;
extern float dewPointInside;
extern float dewPointInsideLastSwicthingTime;

extern float humidityOutside;
extern float temperatureOutside;
extern float dewPointOutside;

void initSensors();
void readSensors();
float calcDewPoint(float humidity, float temperature);
void calcDewPoints();