#pragma once

#include <Arduino.h>
#include <XBee.h>

class Receiver {
public:
    XBee xbee = XBee();

    void setup();

    /**
     * Reads a packet from the XBee.
     * Returns the payload as a hex string (e.g. "A1B2C3") if a valid
     * acknowledged ZB RX packet is received, or an empty string otherwise.
     */
    String ReceiveData();

private:
    XBeeResponse response = XBeeResponse();
    ZBRxResponse rx = ZBRxResponse();
    ModemStatusResponse msr = ModemStatusResponse();
};
