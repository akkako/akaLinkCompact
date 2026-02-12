#ifndef __DRV_CLOCK_H__
#define __DRV_CLOCK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern uint32_t SystemCoreClock;

    extern void SystemInit(void);

#ifdef __cplusplus
}
#endif

#endif
