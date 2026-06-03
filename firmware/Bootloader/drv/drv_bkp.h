#ifndef __DRV_BKP_H__
#define __DRV_BKP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void drv_bkp_init (void);
void drv_bkp_write_reg (uint16_t data);

#ifdef __cplusplus
}
#endif

#endif

