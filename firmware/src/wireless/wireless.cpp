// custom include
#include "wireless.h"

WiFiClient wifiClient;
HTTPClient sender;

// WiFi watchdog / reconnect configuration
// Interval between connectivity checks (human-readable seconds).
// Example: set to 10 for a 10 second check interval.
#define WIFI_CHECK_INTERVAL_SECONDS 10UL
// Derived value in milliseconds for timing APIs
#define WIFI_CHECK_INTERVAL_MS (WIFI_CHECK_INTERVAL_SECONDS * 1000UL)

// Maximum number of failed reconnect cycles before restarting
#define WIFI_RECONNECT_MAX_FAILS 3

// How long to wait (in seconds) for a reconnect attempt to succeed
#define WIFI_RECONNECT_WAIT_SECONDS 3UL
// Derived value in milliseconds used during the reconnect wait loop
#define WIFI_RECONNECT_WAIT_MS (WIFI_RECONNECT_WAIT_SECONDS * 1000UL)

// Configure WiFiManager connect timeout (seconds) to match watchdog check interval
#define WIFI_CONNECT_TIMEOUT_S WIFI_CHECK_INTERVAL_SECONDS

const String FAN_REQUEST_OFF = "/cm?cmnd=Power%20Off";
const String FAN_REQUEST_ON = "/cm?cmnd=Power%20On";
const String FAN_REQUEST_STATUS = "/cm?cmnd=Power";

boolean fanStatusWorkshop;
boolean fanStatusPantry;

void initWIFI()
{
    WiFiManager wifiManager; // Local intialization. Once its business is done, there is no need to keep it around
    // configure connect timeout to match watchdog interval
    wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT_S);
    // wifiManager.resetSettings();//reset saved settings
    // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); //set custom ip for portal

    // fetches ssid and pass from eeprom and tries to connect
    // if it does not connect it starts an access point with the specified name
    // here  "AutoConnectAP"

    if(!wifiManager.autoConnect("Cellar Fan Control")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    } 

    // if you get here you have connected to the WiFi
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Serial.println("WIFI connected.\n");
    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void initFans()
{
    fansOff();
}

// watchdog: periodically ensure WiFi is connected, try reconnects, restart if repeated failures
void checkWiFi()
{
    static unsigned long lastCheck = 0;
    static int failCount = 0;

    if (millis() - lastCheck < WIFI_CHECK_INTERVAL_MS) return;
    lastCheck = millis();

    if (WiFi.status() == WL_CONNECTED) {
        if (failCount) {
            Serial.println("WiFi re-established.");
            failCount = 0;
        }
        return;
    }

    Serial.println("WiFi lost — attempting reconnect...");
    WiFi.reconnect(); // lightweight attempt

    unsigned long start = millis();
    while (millis() - start < WIFI_RECONNECT_WAIT_MS && WiFi.status() != WL_CONNECTED) {
        delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi reconnected.");
        failCount = 0;
    } else {
        failCount++;
        Serial.printf("WiFi reconnect failed (%d)\n", failCount);
        if (failCount >= WIFI_RECONNECT_MAX_FAILS) {
            Serial.println("Exceeded WiFi retries — restarting ESP.");
            delay(100);
            ESP.restart();
        }
    }
}

int fanPowerStatus(String fanName, String request)
{
    int result = -1;
    // don't attempt HTTP requests when WiFi is down
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Skipping HTTP request: WiFi not connected");
        return -1;
    }

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