#include <Arduino.h>
#include <env.h>
#if SENDER_CODE
    #include <sender.h>
#else
    #include <receiver.h>
    #include <RTCManager.h>
    #include <WiFi.h>
    #include <PubSubClient.h>
#endif

#if SENDER_CODE
#else
    RTCManager rtc;
    Receiver receiver;
    WiFiClient wifiClient;
    PubSubClient pubSubClient(wifiClient);
#endif

void setup()
{
    #if SENDER_CODE
        senderSetup();
    #else
        Serial.begin(9600);
        while (!Serial)
            ; // wait for serial port to connect (native USB)
        Wire.begin();
        if (!rtc.begin())
        {
            Serial.println("Couldn't find RTC");
            Serial.flush();
        }

        if (!rtc.isRunning())
        {
            Serial.println("RTC was not running - time set to compile time.");
        }
        receiver.setup();

        // Wifi setup
        // WiFi.config(LOCAL_IP, GATEWAY, SUBNET, DNS);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi. localIP());
        Serial.println(WiFi.gatewayIP());

        pubSubClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    #endif
}

void loop()
{
    #if SENDER_CODE
        senderLoop();
    #else
        if (!pubSubClient.connected())
        {
            while (!pubSubClient.connected())
            {
                Serial.print("Connecting to MQTT...");

                if (pubSubClient.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD))
                {
                    Serial.println("connected");
                }
                else
                {
                    Serial.print("failed, rc=");
                    Serial.println(pubSubClient.state());
                    delay(2000);
                }
            }
        }

        pubSubClient.loop();

        String jsonData;
        jsonData = receiver.ReceiveData();

        if (jsonData.length() > 0)
        {
            jsonData += R"(","timestamp":")" + rtc.getISO() + R"("})";
            pubSubClient.publish("Edusign", jsonData.c_str());
            #if DEBUG
                Serial.println(jsonData);
            #endif
        }
    #endif
}