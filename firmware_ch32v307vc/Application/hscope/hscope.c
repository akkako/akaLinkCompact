#include "hscope.h"
#include "DAP.h"
#include "DAP_config.h"
#include "swd_host.h"
#include <stdio.h>

uint8_t hsc_state = 0;
uint8_t hsc_enable = 0;
uint32_t hsc_data = 0;
uint32_t hsc_addr = 0x20000348;

/**
 * @brief 读取32位内存
 * @param addr 地址
 * @param result 结果指针
 * @return 1 成功
 * @return 0 失败
 */
uint8_t hscope_read_mem_32bit(uint32_t addr, uint32_t *result)
{
    return swd_read_word(addr, result);
}

void hscope_enable(void)
{
    hsc_enable = 1;
}

void hscope_disable(void)
{
    hsc_enable = 0;
}

void hscope_set_addr(uint32_t addr)
{
    hsc_addr = addr;
}

uint32_t hscope_get_data(void)
{
    return hsc_data;
}

void hscope_process(void)
{
    if (hsc_state == 0 && hsc_enable == 0)
    {
        return;
    }

    if (hsc_state == 0 && hsc_enable == 1)
    {
        // init
        swd_fetch_init();
        printf("hscope init\n");
        hsc_state = 1;
    }

    if (hsc_state == 1 && hsc_enable == 0)
    {
        // deinit
        printf("hscope deinit\n");
        hsc_state = 0;
    }

    if (hsc_state == 1 && hsc_enable == 1)
    {
        // process
        hscope_read_mem_32bit(hsc_addr, &hsc_data);
        if (hsc_data != 0x10000000)
        {
            printf("hscope data: 0x%04x\n", hsc_data);
        }
    }
}