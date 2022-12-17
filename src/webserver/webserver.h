#pragma once

// framework include
#include "Arduino.h"

// custom include

// library include
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern ESP8266WebServer server;

// extern const String postForms;

void initWebserver();
void handleRoot();
void handlePlain();
void handleForm();
void handleNotFound();
