#ifndef __HSCOPE_H__
#define __HSCOPE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void hscope_enable(void);
    void hscope_disable(void);
    void hscope_process(void);
    void hscope_set_addr(uint32_t addr);
    uint32_t hscope_get_data(void);

    uint8_t hscope_read_mem_32bit(uint32_t addr, uint32_t *result);

#ifdef __cplusplus
}
#endif

#endif
