// custom include
#include "sensor_BME280.h"

// library include
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"

Adafruit_BME280 bmeInside, bmeOutside;

unsigned long measureInterval = 3 * 60 * 1000; // in millis, default: 3 min
unsigned long measurementTimestamp = 0;

int dewPointThreshold = 4;             // in K, default: 4K
int minInsideTemperatureCutoff = 8;    // in °C, default: 8°C
int minOutsideTemperatureCutoff = -10; // in °C, default: -10°C

float humidityInside;
float temperatureInside;
float dewPointInside;
float dewPointInsideLastSwicthingTime;

float humidityOutside;
float temperatureOutside;
float dewPointOutside;

void initSensors()
{
    // sensors on
    digitalWrite(BME280_POWER_PIN, HIGH);
    delay(25);

    // check indoor sensor status
    bool statusInside = bmeInside.begin(BME280_ADDRESS_INSIDE);

    if (!statusInside)
    {
        Serial.println("ERROR: Could not find the BME280 indoor sensor, check wiring!");
    }
    else
    {
        Serial.println("BME280 indoor sensor up and running");
    }

    // check outdoor sensor status
    bool statusOutside = bmeInside.begin(BME280_ADDRESS_OUTSIDE);

    if (!statusOutside)
    {
        Serial.println("ERROR: Could not find the BME280 outdoor sensor, check wiring!");
    }
    else
    {
        Serial.println("BME280 outdoor sensor up and running");
    }

    digitalWrite(BME280_POWER_PIN, LOW);
}

void readSensors()
{
    // sensors on
    digitalWrite(BME280_POWER_PIN, HIGH);
    delay(25);

    // read indoor values
    humidityInside = -100.0;
    temperatureInside = -100.0;

    bool statusInside = bmeInside.begin(BME280_ADDRESS_INSIDE);

    if (statusInside)
    {
        humidityInside = bmeInside.readHumidity();
        temperatureInside = bmeInside.readTemperature();
    }
    else
    {
        Serial.println("ERROR: Could not find the BME280 indoor sensor. Check wiring!");
    }

    // read outdoor values
    humidityOutside = -100.0;
    temperatureOutside = -100.0;

    bool statusOutside = bmeOutside.begin(BME280_ADDRESS_OUTSIDE);

    if (statusOutside)
    {
        humidityOutside = bmeOutside.readHumidity();
        temperatureOutside = bmeOutside.readTemperature();
    }
    else
    {
        Serial.println("ERROR: Could not find the BME280 outdoor sensor. Check wiring!");
    }

    // sensors off
    digitalWrite(BME280_POWER_PIN, LOW);
}

float calcDewPoint(float humidity, float temperature)
{
    float k;
    k = log(humidity / 100) + (17.62 * temperature) / (243.12 + temperature);
    return 243.12 * k / (17.62 - k);
}

void calcDewPoints()
{
    dewPointInside = -100.0;
    dewPointOutside = -100.0;

    dewPointOutside = calcDewPoint(humidityOutside, temperatureOutside);
    dewPointInside = calcDewPoint(humidityInside, temperatureInside);

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}