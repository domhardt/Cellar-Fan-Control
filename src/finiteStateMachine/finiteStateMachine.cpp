// custom include
#include "finiteStateMachine.h"

// for state machine
const unsigned long VENTILATION_INTERVAL = 30 * 60 * 1000; // in millis, default: 30 min
const unsigned long WAIT_INTERVAL = 90 * 60 * 1000;        // in millis, default: 90 min

unsigned long stateStartTimeStamp = 0;

// enum states
// {
//     NONE,
//     MEASURING,
//     VENTILATING,
//     WAITING
// };

 states state, priorState;

void initFinitStateMachine()
{
    priorState = NONE;
    state = MEASURING;
}

void takeMeasurements()
{
    boolean measurementIntervalExpired = millis() > measurementTimestamp + MEASURE_INTERVAL;

    if (measurementIntervalExpired)
    {
        measurementTimestamp = millis();
        readSensors();
        calcDewPoints();
        logToThingSpeak();
        printDebugInformation();
    }
}

// state methods
void measure()
{
    // initialise stuff on entering a state
    if (state != priorState)
    {
        Serial.println(String("entering state ") + __PRETTY_FUNCTION__);
        priorState = state;
    }

    // do state stuff repeatedly
    takeMeasurements();

    // check for state transition
    boolean humidityDifferenceHigh = dewPointInside - dewPointOutside > DEW_POINT_THRESHOLD;
    boolean temperatureInsideTooLow = temperatureInside < MIN_INSIDE_TEMPERATURE_CUTOFF;
    boolean temperatureOutsideTooLow = temperatureOutside < MIN_OUTSIDE_TEMPERATURE_CUTOFF;

    boolean leaveStateToVentilate = humidityDifferenceHigh && !temperatureInsideTooLow && !temperatureOutsideTooLow;

    if (leaveStateToVentilate)
    {
        state = VENTILATING;
    }

    // clean up on leaving a state
    if (state != priorState)
    {
        Serial.println(String("leaving state ") + __PRETTY_FUNCTION__);
    }
}

void ventilate()
{
    if (state != priorState)
    { // initialise stuff on entering a state
        Serial.println(String("entering state ") + __PRETTY_FUNCTION__);
        priorState = state;
        stateStartTimeStamp = millis();
        dewPointInsideLastSwicthingTime = dewPointInside;
        fansOn();
    }

    // do state stuff repeatedly
    takeMeasurements();

    // check for state transition
    boolean ventilationIntervalExpired = millis() > stateStartTimeStamp + VENTILATION_INTERVAL;
    boolean temperatureInsideTooLow = temperatureInside < MIN_INSIDE_TEMPERATURE_CUTOFF;
    boolean temperatureOutsideTooLow = temperatureOutside < MIN_OUTSIDE_TEMPERATURE_CUTOFF;
    boolean humidityOutsideTooHigh = dewPointOutside > dewPointInsideLastSwicthingTime - (DEW_POINT_THRESHOLD / 2);

    boolean leaveStateToWaiting = ventilationIntervalExpired || temperatureInsideTooLow || temperatureOutsideTooLow || humidityOutsideTooHigh;

    if (leaveStateToWaiting)
    {
        Serial.println(String("ventilationIntervalExpired=") + ventilationIntervalExpired);
        Serial.println(String("temperatureInsideTooLow=") + temperatureInsideTooLow);
        Serial.println(String("humidityOutsideTooHigh=") + humidityOutsideTooHigh);
        state = WAITING;
    }

    // clean up on leaving a state
    if (state != priorState)
    {
        fansOff();
        Serial.println(String("leaving state ") + __PRETTY_FUNCTION__);
    }
}

void wait()
{
    if (state != priorState)
    { // initialise stuff on entering a state
        Serial.println(String("entering state ") + __PRETTY_FUNCTION__);
        priorState = state;
        stateStartTimeStamp = millis();
    }

    // do state stuff repeatedly
    takeMeasurements();

    // check for state transition
    boolean waitIntervalExpired = millis() > stateStartTimeStamp + WAIT_INTERVAL;

    boolean leaveStateToMeasuring = waitIntervalExpired;

    if (leaveStateToMeasuring)
    {
        state = MEASURING;
    }

    // clean up on leaving a state
    if (state != priorState)
    {
        Serial.println(String("leaving state ") + __PRETTY_FUNCTION__);
    }
}

void runFiniteStateMachine()
{
    switch (state)
    {
    case NONE:
        state = MEASURING;
        break;
    case MEASURING:
        measure();
        break;
    case VENTILATING:
        ventilate();
        break;
    case WAITING:
        wait();
        break;
    }
}

String stateString()
{
    const char *stateStr[] = {"NONE", "MEASURING", "VENTILATING", "WAITING"};
    return stateStr[state];
}