#ifndef __DRV_CLOCK_H__ 
#define __DRV_CLOCK_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include <stdint.h>

extern uint32_t SystemCoreClock;          /* System Clock Frequency (Core Clock) */

extern void SystemInit(void);

#ifdef __cplusplus
}
#endif

#endif

