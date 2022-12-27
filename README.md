# Cellar-Fan-Control

## Functionality
- humidity-controlled cellar fan to dry out a cellar
- only vents the cellar if the inside dew point is higher than the putside dew point
- switches fans that are connected to WiFi plugs that run TASMOTA
- runs a website with live "view" of parameters and settings
- uses ThingSpeak for logging

## Hardware
- 1x ESP8266 WiFi microcontroller
- 2x BOSCH BME280 humidity and temperature sensor (pressure is not used)
- 2x TASMOTA-enabled WiFi socket (e.g., SONOFF S26)
- 2x fan (e.g., EBM-PAPST 4650N)
