#ifndef __DRV_PRINT_H__
#define __DRV_PRINT_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void drv_print_init(void);
int drv_print_send(char *buf, int size);

#ifdef __cplusplus
}
#endif

#endif
