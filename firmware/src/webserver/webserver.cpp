// custom include
#include "webserver.h"
#include "html.h"
#include "../config/config.h"
#include "../wireless/wireless.h"
// include for OTA update server
#include <ESP8266HTTPUpdateServer.h>
#include "../logging/secrets.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

void initWebserver()
{
    if (MDNS.begin(WIFI_MDNS_NAME)) // start the mDNS responder for cellar-fan-control.local
    {
        MDNS.addService("http", "tcp", 80);
        Serial.println("MDNS responder started.");
    }
    else
    {
        Serial.println("ERROR: Failed setting up MDNS responder.");
    }

    server.on("/", handleRoot);
    server.on("/save", handleForm);
    server.onNotFound(handleNotFound);

    // Setup web-based OTA update endpoint. LittleFS is initialized in initConfig(), called before initWebserver().
#ifdef SECRET_OTA_PASSWORD
    httpUpdater.setup(&server, "/update", "admin", SECRET_OTA_PASSWORD);
    Serial.println("HTTP OTA update endpoint enabled (authentication: admin / <password>)");
#else
    httpUpdater.setup(&server);
    Serial.println("HTTP OTA update endpoint enabled (no authentication)");
#endif

    server.begin();
    Serial.println("HTTP server started");

    Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void handleRoot()
{
    String head = HTML_header;
    String foot = HTML_footer;
    String formhead = HTML_form_header;
    String formfoot = HTML_form_footer;

    String content = "<h2>Live Information</h2> <p>" + fanInfo() + "<br />" + humidityInfo() + "<br />" + temperatureInfo() + "<br />" + dewPointInfo() + "<br />" +  stateInfo() + "</p>";

    String measureIntervalFormElement = "Measure Interval: <input type=\"number\" name=\"measureInterval\" value=\"" + String(measureInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: " + String(DEFAULT_MEASURE_INTERVAL_MINUTES) + " minutes, minimum: 1 minute)<br />";
    String ventilationIntervalFormElement = "Ventilation Interval: <input type=\"number\" name=\"ventilationInterval\" value=\"" + String(ventilationInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: " + String(DEFAULT_VENTILATION_INTERVAL_MINUTES) + " minutes, minimum: 1 minute)<br />";
    String waitIntervalFormElement = "Wait Interval: <input type=\"number\" name=\"waitInterval\" value=\"" + String(waitInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: " + String(DEFAULT_WAIT_INTERVAL_MINUTES) + " minutes, minimum: 1 minute)<br />";

    String dewPointThresholdFormElement = "Dew Point Threshold: <input type=\"number\" name=\"dewPointThreshold\" value=\"" + String(dewPointThreshold) + "\" min=\"2\"> K (default: " + String(DEFAULT_DEW_POINT_THRESHOLD) + " K, minimum: 2 K, explanation: inside dew point - outside dew point)<br />";
    String minInsideTemperatureCutoffFormElement = "Minimum Inside Cutoff Temperature: <input type=\"number\" name=\"minInsideTemperatureCutoff\" value=\"" + String(minInsideTemperatureCutoff) + "\" min=\"6\"> °C (default: " + String(DEFAULT_MIN_INSIDE_TEMPERATURE_CUTOFF) + " °C, minimum: 6 °C)<br />";
    String minOutsideTemperatureCutoffFormElement = "Minimum Outside Cutoff Temperature: <input type=\"number\" name=\"minOutsideTemperatureCutoff\" value=\"" + String(minOutsideTemperatureCutoff) + "\" min=\"-272\"> °C (default: " + String(DEFAULT_MIN_OUTSIDE_TEMPERATURE_CUTOFF) + " °C)<br />";

    String formcontent = measureIntervalFormElement + ventilationIntervalFormElement + waitIntervalFormElement + dewPointThresholdFormElement + minInsideTemperatureCutoffFormElement + minOutsideTemperatureCutoffFormElement;

    server.send(200, "text/html", head + content + formhead + formcontent + formfoot + foot);
}

void handleForm()
{
    String head = HTML_header;
    String foot = HTML_footer;
    String link = HTML_saved_link;

    if (server.method() != HTTP_POST)
    {
        String errorMessage = "ERROR: Method other than HTTP_POST not allowed.";
        server.send(405, "text/plain", errorMessage);
        Serial.println(errorMessage);
    }
    else
    {
        String valueString;

        if (server.hasArg("measureInterval") && server.arg("measureInterval") != NULL)
        {
            valueString = server.arg("measureInterval");
            measureInterval = valueString.toInt() * 1000 * 60;
        }

        if (server.hasArg("ventilationInterval") && server.arg("ventilationInterval") != NULL)
        {
            valueString = server.arg("ventilationInterval");
            ventilationInterval = valueString.toInt() * 1000 * 60;
        }

        if (server.hasArg("waitInterval") && server.arg("waitInterval") != NULL)
        {
            valueString = server.arg("waitInterval");
            waitInterval = valueString.toInt() * 1000 * 60;
        }

        if (server.hasArg("dewPointThreshold") && server.arg("dewPointThreshold") != NULL)
        {
            valueString = server.arg("dewPointThreshold");
            dewPointThreshold = valueString.toInt();
        }

        if (server.hasArg("minInsideTemperatureCutoff") && server.arg("minInsideTemperatureCutoff") != NULL)
        {
            valueString = server.arg("minInsideTemperatureCutoff");
            minInsideTemperatureCutoff = valueString.toInt();
        }

        if (server.hasArg("minOutsideTemperatureCutoff") && server.arg("minOutsideTemperatureCutoff") != NULL)
        {
            valueString = server.arg("minOutsideTemperatureCutoff");
            minOutsideTemperatureCutoff = valueString.toInt();
        }

        saveConfig();
        server.send(200, "text/html", head + link + foot);
    }
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}