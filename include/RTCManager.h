#pragma once

#include <Arduino.h>
#include "RTClib.h"

class RTCManager {
public:
    /**
     * Initialize the RTC. Calls Wire.begin() and starts the DS1307.
     * Automatically sets the RTC to compile time if it is not running.
     * Returns true on success, false if the RTC was not found.
     */
    bool begin();

    /**
     * Returns true if the RTC oscillator is currently running.
     */
    bool isRunning();

    /**
     * Returns the current date and time as a RTClib DateTime object.
     */
    DateTime now();

    /**
     * Returns the current date and time as an ISO 8601 string.
     * Format: "YYYY-MM-DDTHH:MM:SS"
     */
    String getISO();

    /**
     * Manually set the RTC time.
     */
    void setTime(const DateTime& dt);

private:
    RTC_DS1307 _rtc;

    static const char* _daysOfTheWeek[7];
};
