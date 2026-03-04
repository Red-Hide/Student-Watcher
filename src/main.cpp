#include <Arduino.h>
#include <env.h>
#if SENDER_CODE
    #include <sender.h>
#else
    #include <receiver.h>
    #include <RTCManager.h>
    #include <SDManager.h>
    #include <WiFi.h>
    #include <PubSubClient.h>
#endif

#if SENDER_CODE
#else
    RTCManager rtc;
    Receiver receiver;
    SDManager sdManager;
    WiFiSSLClient wifiClient;
    /* WiFiClient wifiClient; */
    PubSubClient pubSubClient(wifiClient);
#endif

void setup()
{
    #if SENDER_CODE
        senderSetup();
    #else
        Serial.begin(9600);
        while (!Serial); // wait for serial port to connect (native USB)
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

        // SD card setup
        if (!sdManager.begin(SD_CS_PIN, SD_QUEUE_FILE))
        {
            Serial.println("SD card init failed – offline queuing disabled.");
        }
        else
        {
            Serial.println("SD card ready.");
        }

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
        // ---------------------------------------------------------------
        // Always read Zigbee first so no packet is ever dropped regardless
        // of WiFi/MQTT state.
        // ---------------------------------------------------------------
        String jsonData = receiver.ReceiveData();
        bool hasData = (jsonData.length() > 0);
        if (hasData)
        {
            jsonData += R"(","timestamp":")" + rtc.getISO() + R"("})";
        }

        bool wifiConnected = (WiFi.status() == WL_CONNECTED);
        bool mqttConnected = wifiConnected && pubSubClient.connected();

        // ---------------------------------------------------------------
        // If we are offline or MQTT is down, queue the data and return.
        // ---------------------------------------------------------------
        if (!wifiConnected || !mqttConnected)
        {
            if (hasData)
            {
                sdManager.storeMessage(jsonData);
                #if DEBUG
                    Serial.println("[SD] Stored (offline): " + jsonData);
                #endif
            }
        }

        if (!wifiConnected)
            return;

        // ---------------------------------------------------------------
        // WiFi is up – attempt MQTT reconnect with a non-blocking timer
        // so we never block the main loop (and thus Zigbee reads).
        // ---------------------------------------------------------------
        if (!mqttConnected)
        {
            static unsigned long lastMqttAttempt = 0;
            if (millis() - lastMqttAttempt >= 5000)
            {
                lastMqttAttempt = millis();
                Serial.print("Connecting to MQTT...");
                if (pubSubClient.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD))
                {
                    Serial.println("connected");
                    if (sdManager.hasQueuedMessages())
                    {
                        Serial.println("[SD] Flushing offline queue...");
                        if (sdManager.flushQueue(pubSubClient, "Edusign"))
                            Serial.println("[SD] Queue flushed successfully.");
                        else
                            Serial.println("[SD] Some messages could not be sent – will retry.");
                    }
                }
                else
                {
                    Serial.print("failed, rc=");
                    Serial.println(pubSubClient.state());
                }
            }
            return;
        }

        pubSubClient.loop();

        // ---------------------------------------------------------------
        // MQTT connected – publish live data (or queue on failure).
        // ---------------------------------------------------------------
        if (hasData)
        {
            if (!pubSubClient.publish("Edusign", jsonData.c_str()))
            {
                sdManager.storeMessage(jsonData);
                #if DEBUG
                    Serial.println("[SD] Publish failed, stored: " + jsonData);
                #endif
            }
            else
            {
                #if DEBUG
                    Serial.println(jsonData);
                #endif
            }
        }
    #endif
}
