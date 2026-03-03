#include "SDManager.h"

bool SDManager::begin(uint8_t csPin, const char* queueFile)
{
    _queueFile = queueFile;

    if (!SD.begin(csPin))
    {
        return false;
    }
    return true;
}

void SDManager::storeMessage(const String& jsonMessage)
{
    // FILE_WRITE opens with O_APPEND – data is added at the end of the file.
    File f = SD.open(_queueFile, FILE_WRITE);
    if (f)
    {
        f.println(jsonMessage);
        f.close();
    }
}

bool SDManager::hasQueuedMessages()
{
    if (!SD.exists(_queueFile))
        return false;

    File f = SD.open(_queueFile, FILE_READ);
    if (!f)
        return false;

    bool hasData = (f.size() > 0);
    f.close();
    return hasData;
}

bool SDManager::flushQueue(PubSubClient& client, const char* topic)
{
    if (!SD.exists(_queueFile))
        return true; // Nothing to flush.

    File f = SD.open(_queueFile, FILE_READ);
    if (!f)
        return false;

    // Collect all lines into a temporary buffer so we can close the file
    // before trying to delete it (FAT requires the file to be closed first).
    const uint8_t MAX_MESSAGES = 64;
    String messages[MAX_MESSAGES];
    uint8_t count = 0;

    while (f.available() && count < MAX_MESSAGES)
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() > 0)
        {
            messages[count++] = line;
        }
    }
    f.close();

    // Publish each message.
    for (uint8_t i = 0; i < count; i++)
    {
        if (!client.publish(topic, messages[i].c_str()))
        {
            // Publish failed – re-store the messages that were not yet sent
            // (from index i onwards) into a fresh queue file.
            SD.remove(_queueFile);
            for (uint8_t j = i; j < count; j++)
            {
                storeMessage(messages[j]);
            }
            return false;
        }
    }

    // All messages sent – remove the queue file.
    SD.remove(_queueFile);
    return true;
}
