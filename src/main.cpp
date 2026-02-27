#include <Arduino.h>
#include <env.h>
#if SENDER_CODE
    #include <sender.h>
#else
    #include <receiver.h>
    #include <RTCManager.h>
#endif

#if SENDER_CODE
#else
    RTCManager rtc;
    Receiver receiver;
#endif

void setup()
{
    #if SENDER_CODE
        senderSetup();
    #else
        Serial.begin(9600);
        while (!Serial); // wait for serial port to connect (native USB)
        Wire.begin();
        if (!rtc.begin()) {
            Serial.println("Couldn't find RTC");
            Serial.flush();
        }

        if (!rtc.isRunning()) {
            Serial.println("RTC was not running - time set to compile time.");
        }
        receiver.setup();
    #endif
}

void loop()
{
    #if SENDER_CODE
        senderLoop();
    #else
        String data;
        data = receiver.ReceiveData();
    #endif
}