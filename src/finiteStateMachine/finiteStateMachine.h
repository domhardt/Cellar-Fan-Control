#pragma once

// framework include
#include "Arduino.h"

// custom include
// #include "../sensor/sensor_DHT22.h"
#include "../sensor/sensor_BME280.h"
#include "../logging/logging.h"

extern unsigned long ventilationInterval;
extern unsigned long waitInterval;
extern unsigned long stateStartTimeStamp;

enum states
{
    NONE,
    MEASURING,
    VENTILATING,
    WAITING
};

extern states state, priorState;

void initFinitStateMachine ();
void takeMeasurements ();
void measure ();
void ventilate ();
void wait();
void runFiniteStateMachine();

String stateString ();