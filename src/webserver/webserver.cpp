// custom include
#include "webserver.h"

ESP8266WebServer server(80);

const String postForms = "<html>\
  <head>\
    <title>Cellar Fan Control</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Cellar Fan Control</h1>\
    <form action=\"/submitPage/\">\
        First name:<br><input type=\"text\" name=\"firstname\" value=\VENTILATION_INTERVAL\><br>\
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
    if (MDNS.begin("CellarFanControl"))
    {
        Serial.println("MDNS responder started");
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
    server.send(200, "text/html", postForms);
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
 server.send(200, "text/html", s); //Send web page
}

void handleReset()
{
}
