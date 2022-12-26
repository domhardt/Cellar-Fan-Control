// custom include
#include "webserver.h"
#include "html.h"

ESP8266WebServer server(80);

const String postForms = "<html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <title>Cellar Fan Control</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Cellar Fan Control</h1>\
    <form action=\"/submitPage/\">\
        First name:<br><input type=\"text\" name=\"firstname\" value=\ventilationInterval\><br>\
        Last name:<br><input type=\"text\" name=\"lastname\" value=\"Mouse\"><br>\
        <input type=\"submit\" value=\"Submit\">\
    </form>\
    // <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
    //   <input type=\"text\" name=\'{\"hello\": \"world\", \"trash\": \"\' value=\'\"}\'><br>\
    //   <input type=\"submit\" value=\"Submit\">\
    // </form>\
    // <h1>POST form data to /postform/</h1><br>\
    // <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
    //   <input type=\"text\" name=\"hello\" value=\"world\"><br>\
    //   <input type=\"submit\" value=\"Submit\">\
    // </form>\
  </body>\
</html>";

void initWebserver()
{
    if (MDNS.begin("cellar-fan-control")) // start the mDNS responder for cellar-fan-control.local
    {
        Serial.println("MDNS responder started");
    }
    else
    {
        Serial.println("ERROR setting up MDNS responder!");
    }

    server.on("/", handleRoot);
    server.on("/postplain/", handlePlain);
    server.on("/postform/", handleForm);
    server.onNotFound(handleNotFound);
    server.on("/submitPage/", handleSubmit);

    server.begin();
    Serial.println("HTTP server started");
}

void handleRoot()
{
    String head = HTML_header;
    String foot = HTML_footer;
    String formhead = HTML_form_header;
    String formfoot = HTML_form_footer;

    String measureIntervalForm = "Measure Interval: <input type=\"number\" name=\"measureInterval\" value=\"" + String(measureInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 3 minutes)<br />";
    String ventilationIntervalForm = "Ventilation Interval: <input type=\"number\" name=\"ventilationInterval\" value=\"" + String(ventilationInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 30 minutes)<br />";
    String waitIntervalForm = "Wait Interval: <input type=\"number\" name=\"waitInterval\" value=\"" + String(waitInterval / 1000 / 60) + "\" min=\"1\"> minutes (default: 90 minutes)<br />";

    String dewPointThresholdForm = "Dew Point Threshold: <input type=\"number\" name=\"dewPointThreshold\" value=\"" + String(dewPointThreshold) + "\" min=\"1\" step=\"any\"> Kelvin (default: 4 Kelvin)<br />";
    String minInsideTemperatureCutoffForm = "Minimum Inside Cutoff Temperature: <input type=\"number\" name=\"minInsideTemperatureCutoff\" value=\"" + String(minInsideTemperatureCutoff) + "\" min=\"6\" step=\"any\"> Grad Celsius (default: 8 Grad Celsius, minimum: 6 Grad Celsius)<br />";
    String minOutsideTemperatureCutoffForm = "Minimum Outside Cutoff Temperature: <input type=\"number\" name=\"minOutsideTemperatureCutoff\" value=\"" + String(minOutsideTemperatureCutoff) + "\" step=\"any\"> Grad Celsius (default: -10 Grad Celsius)<br />";

    String formcontent = measureIntervalForm + ventilationIntervalForm + waitIntervalForm + dewPointThresholdForm + minInsideTemperatureCutoffForm + minOutsideTemperatureCutoffForm;

    server.send(200, "text/html", head + formhead + formcontent + formfoot + foot);
}

void handlePlain()
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
    else
    {
        server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    }
}

void handleForm()
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
    else
    {
        String message = "POST form was:\n";
        for (uint8_t i = 0; i < server.args(); i++)
        {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }
        server.send(200, "text/plain", message);
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

void handleSubmit()
{
    String firstName = server.arg("firstname");
    String lastName = server.arg("lastname");

    Serial.print("First Name:");
    Serial.println(firstName);

    Serial.print("Last Name:");
    Serial.println(lastName);

    String s = "<a href='/'> Go Back </a>";
    server.send(200, "text/html", s); // Send web page
}

void handleReset()
{
}
