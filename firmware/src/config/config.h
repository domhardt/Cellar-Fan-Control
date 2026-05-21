#pragma once

constexpr unsigned long DEFAULT_MEASURE_INTERVAL_MINUTES = 3UL;
constexpr unsigned long DEFAULT_VENTILATION_INTERVAL_MINUTES = 60UL;
constexpr unsigned long DEFAULT_WAIT_INTERVAL_MINUTES = 60UL;
constexpr int DEFAULT_DEW_POINT_THRESHOLD = 5;
constexpr int DEFAULT_MIN_INSIDE_TEMPERATURE_CUTOFF = 10;
constexpr int DEFAULT_MIN_OUTSIDE_TEMPERATURE_CUTOFF = -10;

void initConfig();
void saveConfig();
void loadConfig();
