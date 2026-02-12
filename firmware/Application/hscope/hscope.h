#ifndef __HSCOPE_H__
#define __HSCOPE_H__

#include <stdint.h>

int8_t hscope_init (uint32_t swd_speed, uint32_t fetch_speed);

int8_t hscope_connect (void);

int8_t hscope_start_fetch (void);

int8_t hscope_fetch_data (void);

int8_t hscope_disconnect (void);


void hscope_test (void);

#endif
