#include <sender.h>
#include <Arduino.h>

void setup()
{
#if SENDER_CODE
    senderSetup();
#endif
}

void loop()
{
#if SENDER_CODE
    senderLoop();
#endif
}