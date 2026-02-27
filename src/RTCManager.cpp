#include "RTCManager.h"
#include <Wire.h>

const char* RTCManager::_daysOfTheWeek[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

bool RTCManager::begin() {
    if (!_rtc.begin()) {
        return false;
    }
    if (!_rtc.isrunning()) {
        // Set to compile-time date/time if the oscillator is stopped
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return true;
}

bool RTCManager::isRunning() {
    return _rtc.isrunning();
}

DateTime RTCManager::now() {
    return _rtc.now();
}

String RTCManager::getISO() {
    DateTime dt = _rtc.now();

    // Format: YYYY-MM-DDTHH:MM:SS
    char buf[20];
    snprintf(buf, sizeof(buf),
        "%04u-%02u-%02uT%02u:%02u:%02u",
        dt.year(), dt.month(), dt.day(),
        dt.hour(), dt.minute(), dt.second()
    );
    return String(buf);
}

void RTCManager::setTime(const DateTime& dt) {
    _rtc.adjust(dt);
}
