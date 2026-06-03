#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include <stdint.h>

extern void drv_flash_read(uint8_t *buff, uint32_t addr, uint32_t length);
extern int drv_flash_write(uint8_t *buff, uint32_t addr, uint32_t length);



#endif
