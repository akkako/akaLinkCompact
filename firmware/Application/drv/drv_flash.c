#include "drv_flash.h"
#include "ch32v30x_flash.h"
#include <string.h>

#define FLASH_PAGE_SIZE (256)

#define MSC_START_ADDR (0x08017000)
#define MSC_BLOCK_NUM (72)
#define MSC_BLOCK_SIZE (512)

uint32_t buf[MSC_BLOCK_SIZE / 4];

void drv_msc_read (uint32_t sector, uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        data[i] = *(uint8_t *)(MSC_START_ADDR + sector * MSC_BLOCK_SIZE + i);
    }
}

void drv_msc_write (uint32_t sector, uint8_t *data, uint32_t len) {

    memcpy (buf, data, MSC_BLOCK_SIZE);

    FLASH_Unlock_Fast();
    FLASH_Access_Clock_Cfg (FLASH_Access_SYSTEM_HALF);

    uint32_t op_block = len / FLASH_PAGE_SIZE;

    uint32_t offset_addr = MSC_START_ADDR + sector * MSC_BLOCK_SIZE;

    for (uint32_t i = 0; i < op_block; i += 1) {
        FLASH_ErasePage_Fast (offset_addr + i * FLASH_PAGE_SIZE);
        memcpy (buf, &data[i * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
        FLASH_ProgramPage_Fast (offset_addr + i * FLASH_PAGE_SIZE, buf);
    }

    FLASH_Lock_Fast();
}