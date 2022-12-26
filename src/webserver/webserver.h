#pragma once

// framework include
#include "Arduino.h"

// custom include
#include "../finiteStateMachine/finiteStateMachine.h"
#include "../sensor/sensor_BME280.h"

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
void handleSubmit();
void handleReset();
