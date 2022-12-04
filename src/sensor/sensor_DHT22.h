#pragma once

// for DTH sensors
#define DHT_TYPE DHT22

#define DHT_INSIDE_PIN 5
#define DHT_OUTSIDE_PIN 12
#define DHT_POWER_PIN 2

#define DEW_POINT_THRESHOLD 4.0              // in K
#define MIN_INSIDE_TEMPERATURE_CUTOFF 8.0    // in °C
#define MIN_OUTSIDE_TEMPERATURE_CUTOFF -10.0 // in °C

#define MEASURE_INTERVAL 180000 // 3 * 60 * 1000; // in millis, default: 3 min

extern unsigned long measurementTimestamp;

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