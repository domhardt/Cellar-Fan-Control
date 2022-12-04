// custom include
#include "sensor_DHT22.h"

// library include
#include "DHT.h"

DHT dhtInside(DHT_INSIDE_PIN, DHT_TYPE);
DHT dhtOutside(DHT_OUTSIDE_PIN, DHT_TYPE);

float humidityInside;
float temperatureInside;
float dewPointInside;
float dewPointInsideLastSwicthingTime;

float humidityOutside;
float temperatureOutside;
float dewPointOutside;

unsigned long measurementTimestamp = 0;

void initSensors()
{
    pinMode(DHT_POWER_PIN, OUTPUT);
    digitalWrite(DHT_POWER_PIN, LOW);
}

void readSensors()
{
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