#pragma once

#include <Arduino.h>
#include <SD.h>
#include <PubSubClient.h>

/**
 * Manages offline message queuing to an SD card.
 *
 * Messages are stored one per line in a plain-text file on the SD card.
 * When the connection is restored, call flushQueue() to resend all stored
 * messages and clear the file.
 */
class SDManager {
public:
    /**
     * Initialise the SD library.
     * @param csPin      Chip-select pin wired to the SD shield (D4 for Seeed
     *                   Studio SD Shield v4.0, D10 for older versions).
     * @param queueFile  Filename to use for the offline message queue.
     * @return true on success, false if the SD card could not be initialised.
     */
    bool begin(uint8_t csPin, const char* queueFile);

    /**
     * Append a JSON message to the on-card queue file.
     * @param jsonMessage  Complete JSON string to store.
     */
    void storeMessage(const String& jsonMessage);

    /**
     * Returns true if the queue file exists and contains at least one message.
     */
    bool hasQueuedMessages();

    /**
     * Read every line from the queue file and publish it to the MQTT broker.
     * If ALL messages publish successfully the queue file is deleted.
     * If any publish call fails the function stops early and keeps the file
     * so that the remaining (and failed) messages are retried next time.
     *
     * @param client  A connected PubSubClient instance.
     * @param topic   MQTT topic to publish to.
     * @return true if all messages were sent and the queue was cleared.
     */
    bool flushQueue(PubSubClient& client, const char* topic);

private:
    const char* _queueFile;
};
