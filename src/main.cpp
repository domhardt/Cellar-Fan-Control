/*
 * Cellar Fan Control
 *
 * implementation for ESP8266
 * uses two DHT22 temperature and humidity sensors
 * controls two WIFI sockets running TASMOTA firmware (e.g., Sonoff S2x)
 * logs data to thingspeak
 * logic based on finite state machine (FSM)
 *
 * by Michael Domhardt
 */

// framework include
#include "Arduino.h"

// custom include
// #include "sensor/sensor_DHT22.h"
#include "sensor/sensor_BME280.h"
#include "wireless/wireless.h"
#include "finiteStateMachine/finiteStateMachine.h"
#include "logging/logging.h"
#include "webserver/webserver.h"

void setup()
{
  initSerial();
  initWIFI();
  initSensors();
  initFans();
  initThingSpeak();
  initFinitStateMachine();
  initWebserver();

  Serial.println(String("finished function ") + __PRETTY_FUNCTION__);
}

void loop()
{
  runFiniteStateMachine();
  server.handleClient();
}