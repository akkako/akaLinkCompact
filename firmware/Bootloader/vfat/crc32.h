#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdint.h>

static inline uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    crc ^= 0xffffffff;
    while (len--)
    {
        crc ^= *buf++;
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

#endif
