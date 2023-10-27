#include "circularBuffer.h"
#include "../logger/logger.h"
#include <stdbool.h>

int getBufferFreeSpace(buffer *buff)
{
    return BUFFERSIZE - buff->count;
}

int getBufferOccupiedSpace(buffer *buff)
{
    return buff->count;
}

int isBufferFull(buffer *buff)
{
    return buff->count == BUFFERSIZE;
}

int isBufferEmpty(buffer *buff)
{
    return buff->count == 0;
}

void writeDataToBuffer(buffer *buff, char * const src, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (isBufferFull(buff))
        {
            log(FATAL, "%s", "Buffer is full, this shouldnt happen");
        }
        buff->buffer[buff->writePtr] = src[i];
        buff->writePtr = (buff->writePtr + 1) % BUFFERSIZE;
        buff->count++;
    }
}

int readDataFromBuffer(buffer *buff, char *dest, int len)
{
    int bytesRead = 0;

    bool readingData = true;
    for (int i = 0; readingData && i < len; i++)
    {
        if (isBufferEmpty(buff))
        {
            readingData = false;
            continue;
        }

        dest[i] = buff->buffer[buff->readPtr];
        buff->readPtr = (buff->readPtr + 1) % BUFFERSIZE;
        buff->count--;
        bytesRead++;
    }

    return bytesRead;
}