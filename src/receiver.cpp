//
// Created by guigui on 25/02/2026.
//

#include "receiver.h"

void Receiver::setup()
{
    Serial1.begin(9600);
    xbee.begin(Serial1);
}

String Receiver::ReceiveData()
{
    xbee.readPacket();

    if (xbee.getResponse().isAvailable())
    {

        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
        {
            xbee.getResponse().getZBRxResponse(rx);

            if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED)
            {
                XBeeAddress64 remoteAddress = rx.getRemoteAddress64();

                uint32_t msb = remoteAddress.getMsb();
                uint32_t lsb = remoteAddress.getLsb();

                String zigBeeIdentifier = String(msb, HEX) + String(lsb, HEX);
                zigBeeIdentifier.toUpperCase();

                // Build hex string from payload
                String nfcTagId;
                for (int i = 0; i < rx.getDataLength(); i++)
                {
                    if (rx.getData(i) < 0x10)
                        nfcTagId += "0";
                    nfcTagId += String(rx.getData(i), HEX);
                    nfcTagId.toUpperCase();
                }

                String resultJson = R"({"zigBeeId":")" + zigBeeIdentifier +
                R"(","nfcTagId":")" + nfcTagId;

                return resultJson;
            }
            else
            {
                // Received but sender didn't get an ACK
                return "";
            }
        }
        else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE)
        {
            xbee.getResponse().getModemStatusResponse(msr);

            if (msr.getStatus() == ASSOCIATED)
            {
                // Zigbee associated with a network
            }
            else if (msr.getStatus() == DISASSOCIATED)
            {
                // Zigbee dissociated from network
            }
            else
            {
                // another status
            }
        }
        else
        {
            // not something we were expecting
        }
    }
    else if (xbee.getResponse().isError())
    {
        // error reading packet
    }

    return "";
}
