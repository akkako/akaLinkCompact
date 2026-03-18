#ifndef __DRV_SYSTICK_H__
#define __DRV_SYSTICK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void drv_systick_init(void);
uint32_t drv_systick_millis(void);
void drv_systick_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif

