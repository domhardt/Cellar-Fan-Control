#pragma once

// framework include
#include "Arduino.h"

// library include
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

// for WIFI Manager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

// for HTTP requests
#include <ESP8266HTTPClient.h>

extern const String FAN_REQUEST_OFF;
extern const String FAN_REQUEST_ON;
extern const String FAN_REQUEST_STATUS;

extern boolean fanStatusWorkshop;
extern boolean fanStatusPantry;

extern WiFiClient wifiClient;

void initWIFI();
void initFans();

int fanPowerStatus(String fanName, String request);
void fansOn();
void fansOff();