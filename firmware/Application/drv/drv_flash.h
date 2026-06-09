#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void drv_msc_read (uint32_t sector, uint8_t *data, uint32_t len);
void drv_msc_write (uint32_t sector, uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
