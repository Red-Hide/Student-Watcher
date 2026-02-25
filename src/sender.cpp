
#include <sender.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>
#include <Arduino_LED_Matrix.h>
#include <XBee.h>

#pragma region Animations class

class AnimationUtil
{
public:
    AnimationUtil(ArduinoLEDMatrix &m) : matrix(m) {}

    AnimationUtil *play(AnimationType anim)
    {
        switch (anim)
        {
        case AnimationType::LOADING:
            matrix.loadSequence(LEDMATRIX_ANIMATION_LOAD);
            break;
        case AnimationType::ANIM_SUCCESS:
            matrix.loadSequence(LEDMATRIX_ANIMATION_CHECK);
            break;
        case AnimationType::ERROR:
            matrix.loadSequence(LEDMATRIX_ANIMATION_BUG);
            break;
        case AnimationType::HEARTBEAT:
            matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT_LINE);
            break;
        }
        matrix.play(true);
        return this;
    }

    AnimationUtil *delayMs(unsigned long ms)
    {
        delay(ms);
        return this;
    }

    void playSuccess()
    {
        this->play(AnimationType::ANIM_SUCCESS)
            ->delayMs(2000)
            ->play(AnimationType::HEARTBEAT)
            ->delayMs(2000);
    }

    void playError()
    {
        this->play(AnimationType::ERROR)
            ->delayMs(2000)
            ->play(AnimationType::HEARTBEAT)
            ->delayMs(2000);
    }

private:
    ArduinoLEDMatrix &matrix;
};

ArduinoLEDMatrix matrix;
AnimationUtil animationUtil(matrix);

#pragma endregion

#pragma region Main logic

SoftwareSerial SWSerial(2, 3);
PN532_SWHSU pn532swhsu(SWSerial);
PN532 nfc(pn532swhsu);
XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4224e3f9);
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

void senderSetup(void)
{
    Serial.begin(115200);
    Serial1.begin(9600);
    xbee.setSerial(Serial1);

    nfc.begin();
    matrix.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    {
        Serial.println("PN532 not detected");
        animationUtil.play(AnimationType::ERROR);
    }

    animationUtil.play(AnimationType::HEARTBEAT);

    nfc.SAMConfig();
}

void senderLoop(void)
{
    uint8_t uid[8];
    uint8_t uidLength;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength))
    {
        animationUtil.play(AnimationType::LOADING);

        String payload = "";

        for (uint8_t i = 0; i < uidLength; i++)
        {
            if (uid[i] < 0x10)
                payload += "0";
            payload += String(uid[i], HEX);
            payload.toUpperCase();
        }

        Serial.println(payload);

        uint8_t buffer[payload.length() + 1];
        payload.getBytes(buffer, payload.length() + 1);

        ZBTxRequest zbTx(addr64, uid, uidLength);
        xbee.send(zbTx);

        if (xbee.readPacket(500))
        {
            if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
            {
                xbee.getResponse().getZBTxStatusResponse(txStatus);
                if (txStatus.getDeliveryStatus() == SUCCESS)
                {
                    animationUtil.playSuccess();
                }
                else
                {
                    animationUtil.playError();
                }
            }
        }
        else
        {
            animationUtil.playError();
        }
    }
}

#pragma endregion