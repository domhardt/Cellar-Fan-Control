// custom include
#include "webserver.h"
#include "html.h"

ESP8266WebServer server(80);

void initWebserver()
{
    if (MDNS.begin("cellar-fan-control")) // start the mDNS responder for cellar-fan-control.local
    {
        Serial.println("MDNS responder started.");
    }
    else
    {
        Serial.println("ERROR: Failed setting up MDNS responder.");
    }

    server.on("/", handleRoot);
    server.on("/save", handleForm);
    server.onNotFound(handleNotFound);

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

    String measureIntervalFormElement = "Measure Interval: <input type=\"number\" name=\"measureInterval\" value=\"" + String(measureInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 3 minutes, minimum: 1 minute)<br />";
    String ventilationIntervalFormElement = "Ventilation Interval: <input type=\"number\" name=\"ventilationInterval\" value=\"" + String(ventilationInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 30 minutes, minimum: 1 minute)<br />";
    String waitIntervalFormElement = "Wait Interval: <input type=\"number\" name=\"waitInterval\" value=\"" + String(waitInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 90 minutes, minimum: 1 minute)<br />";

    String dewPointThresholdFormElement = "Dew Point Threshold: <input type=\"number\" name=\"dewPointThreshold\" value=\"" + String(dewPointThreshold) + "\" min=\"2\"> K (default: 4 K, minimum: 2 K, explanation: inside dew point - outside dew point<br />";
    String minInsideTemperatureCutoffFormElement = "Minimum Inside Cutoff Temperature: <input type=\"number\" name=\"minInsideTemperatureCutoff\" value=\"" + String(minInsideTemperatureCutoff) + "\" min=\"6\"> °C (default: 8 °C, minimum: 6 °C)<br />";
    String minOutsideTemperatureCutoffFormElement = "Minimum Outside Cutoff Temperature: <input type=\"number\" name=\"minOutsideTemperatureCutoff\" value=\"" + String(minOutsideTemperatureCutoff) + "\" min=\"-272\"> °C (default: -10 °C)<br />";

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