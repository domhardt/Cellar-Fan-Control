// custom include
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

void initThingSpeak()
{
    ThingSpeak.begin(wifiClient); // Initialize ThingSpeak
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

void logToThingSpeak()
{
    ThingSpeak.setStatus(String("State: ") + stateString());

    ThingSpeak.setField(1, fanStatusWorkshop);
    ThingSpeak.setField(2, fanStatusPantry);
    ThingSpeak.setField(3, humidityInside);
    ThingSpeak.setField(4, humidityOutside);
    ThingSpeak.setField(5, temperatureInside);
    ThingSpeak.setField(6, temperatureOutside);
    ThingSpeak.setField(7, dewPointInside);
    ThingSpeak.setField(8, dewPointOutside);

    int statusMessage = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);

    if (statusMessage == 200)
    {
        Serial.println("Thingspeak channel update successful.");
    }
    else
    {
        Serial.println("ERROR: Problem updating channel. HTTP error code " + String(statusMessage));
    }

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

String fanInfo()
{
    return String("Fan Status Workshop = ") + fanStatusWorkshop + String(" ; Fan Status Pantry = ") + fanStatusPantry + String(" [0/1]");
}

String humidityInfo()
{
    return String("Humidity Inside = ") + humidityInside + String(" % ; Humidity Outside = ") + humidityOutside + String(" %");
}
String temperatureInfo()
{
    return String("Temperature Inside = ") + temperatureInside + String(" °C ; Temperature Outside = ") + temperatureOutside  + String(" °C");
}
String dewPointInfo()
{
    double dewPointDifference = dewPointInside - dewPointOutside;
    return String("Dew Point Inside = ") + dewPointInside + String(" °C ; Dew Point Outside = ") + dewPointOutside + String(" °C ; Dew Point Difference = ") + dewPointDifference   + String(" K");
}