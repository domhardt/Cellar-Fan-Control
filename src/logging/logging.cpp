// custom include
#include "logging.h"
#include "logging.h"

// for serial
void initSerial()
{
    Serial.begin(74480);
    delay(500);

    while (!Serial)
        ;
    delay(500);

    Serial.println();
    Serial.println("\nNAME: Humidity Controlled Cellar Fan");
    Serial.println("FILE: " __FILE__);
    Serial.println("VERSION: " __DATE__ ", " __TIME__);
    Serial.println("AUTHOR: Michael Domhardt\n");

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void printValuesToSerial(String location, float humidity, float temperature, float dewPoint)
{
    Serial.print(location + String(": "));
    Serial.print(String("temperature = ") + temperature + String("°C, "));
    Serial.print(String("humidity = ") + humidity + String("%, "));
    Serial.print(String("dewPoint = ") + dewPoint + String("°C"));
    Serial.println();
}

void printDebugInformation()
{
    Serial.println(String("fanStatusWorkshop = ") + fanStatusWorkshop + String(", fanStatusPantry = ") + fanStatusPantry);
    printValuesToSerial("INDOOR ", humidityInside, temperatureInside, dewPointInside);
    printValuesToSerial("OUTDOOR", humidityOutside, temperatureOutside, dewPointOutside);
}