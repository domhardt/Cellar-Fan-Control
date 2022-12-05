// custom include
#include "wireless.h"

WiFiClient wifiClient;
HTTPClient sender;

const String FAN_REQUEST_OFF = "/cm?cmnd=Power%20Off";
const String FAN_REQUEST_ON = "/cm?cmnd=Power%20On";
const String FAN_REQUEST_STATUS = "/cm?cmnd=Power";

boolean fanStatusWorkshop;
boolean fanStatusPantry;

void initWIFI()
{
    WiFiManager wifiManager; // Local intialization. Once its business is done, there is no need to keep it around
    // wifiManager.resetSettings();//reset saved settings
    // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); //set custom ip for portal

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"
    wifiManager.autoConnect("AutoConnectAP"); // and goes into a blocking loop awaiting configuration
    // wifiManager.autoConnect();    //or use this for auto generated name ESP + ChipID

    Serial.println("WIFI connected.\n)"); // if you get here you have connected to the WiFi
    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void initFans()
{
    fansOff();
}

int fanPowerStatus(String fanName, String request)
{
    int result = -1;

    String httpRequest = "http://" + fanName + request;
    if (sender.begin(wifiClient, httpRequest))
    {                                // establish connection, initialise request
        int httpCode = sender.GET(); // HTTP-Code of the response

        if (httpCode > 0)
        { // request sent and server responded
            if (httpCode == HTTP_CODE_OK)
            {                                        // response was OK aka code 200
                String payload = sender.getString(); // response string
                Serial.println(fanName + String("PowerStatus = ") + payload);

                if (payload.indexOf("ON") > 0)
                {
                    result = 1;
                }
                else if (payload.indexOf("OFF") > 0)
                {
                    result = 0;
                }
                else
                {
                    Serial.println("WARNING: fan power status could not be determind.");
                }
            }
        }
        else
        { // HTTP error handling
            Serial.println(String("TASMOTA HTTP-Error: ") + sender.errorToString(httpCode).c_str());
        }

        sender.end(); // end request and terminate conection
    }
    else
    {
        Serial.printf("TASMOTA WARNING: HTTP connection could not be established.");
    }

    return result;
}

void fansOn()
{
    fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_ON);
    fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_ON);

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void fansOff()
{
    fanStatusWorkshop = fanPowerStatus("cellarfanworkshop", FAN_REQUEST_OFF);
    fanStatusPantry = fanPowerStatus("cellarfanpantry", FAN_REQUEST_OFF);

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}