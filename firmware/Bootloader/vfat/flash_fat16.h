#ifndef __FLASH_FAT16_H__
#define __FLASH_FAT16_H__

#include <stdint.h>
#include <stdbool.h>

#define VFAT16_CONF_SECTOR_SIZE (2048)

#define VFAT16_FLASH_START_ADDR (0x08007000)
#define VFAT16_FLASH_SIZE (100 * 1024)

#ifdef __cplusplus
extern "C"
{
#endif

    uint32_t flash_fat16_write(uint32_t fat_lbk, uint8_t *data, uint32_t len);
    uint32_t flash_fat16_read(uint32_t fat_lbk, uint8_t *data, uint32_t len);
    void fat16_file_init(void);

#ifdef __cplusplus
}
#endif

#endif
