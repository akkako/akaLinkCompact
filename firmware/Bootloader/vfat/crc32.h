#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdint.h>

uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len);

#endif
