#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void drv_timer_pulse_init (uint16_t period);
void drv_timer_pulse_enable (void);
void drv_timer_pulse_disable (void);
void drv_timer_blank_init (uint16_t period);
void drv_timer_blank_enable (void);
void drv_timer_blank_disable (void);
void drv_timer_count_init (void);
void drv_timer_count_enable (void);
void drv_timer_count_disable (void);
uint16_t drv_timer_count_getcnt (void);
void drv_timer_count_clearcnt (void);

#ifdef __cplusplus
}
#endif

#endif
