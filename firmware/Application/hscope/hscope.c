#include "hscope.h"
#include "DAP.h"
#include "DAP_config.h"
#include "swd_host.h"

enum
{
    HSCOPE_FETCH_MODE_BLOCK = 0,    // 整块读取模式，连读一整块数据
    HSCOPE_FETCH_MODE_LIST = 1,     // 链表读取模式，依次读取分散数据
    HSCOPE_FETCH_MODE_PINGPONG = 2, // 乒乓读取模式，与标志位进行同步读取缓冲区
};

enum
{
    HSCOPE_REPORT_MODE_RAW = 0,       // 原始上报模式，原始小端对齐数据上报
    HSCOPE_REPORT_MODE_FLOAT = 1,     // 浮点数上传模式，适用于VOFA+
    HSCOPE_REPORT_MODE_FIREWATER = 2, // 格式化上报模式，适用于VOFA+
};

typedef struct
{
    uint32_t enable : 8;
    uint32_t type : 8;
    uint32_t padding : 16;
    uint32_t addr;
} fetch_info_t;

uint32_t result = 0;

struct hscope_t
{
    uint32_t set_fetch_speed;
    uint32_t set_fetch_mode;
    uint32_t set_report_mode;
    uint32_t set_swd_speed;

    uint32_t act_fetch_speed;
    uint32_t act_swd_speed;
} hscope;

int8_t hscope_init(uint32_t swd_speed, uint32_t fetch_speed)
{
    hscope.set_fetch_speed = fetch_speed;
    hscope.set_swd_speed = swd_speed;

    swd_init();
    // set swd speed
    Set_Clock_Delay(swd_speed);

    // TODO
    // set actual timer
    return 1;
}

int8_t hscope_connect(void)
{
    uint8_t res = 0;
    res = JTAG2SWD();
    if (res == 0)
    {
        return 0;
    }

    res = swd_fetch_init();
    if (res == 0)
    {
        return 0;
    }
    return 1;
}

int8_t hscope_start_fetch(void)
{
    return 0;
}

int8_t hscope_fetch_data(void)
{
    return swd_read_word(0x20000068, &result);
}

int8_t hscope_disconnect(void)
{
    return 0;
}

void hscope_test(void)
{
    int8_t res = 0;
    // init
    res = hscope_init(10000000, 0);
    // init check
    if (res == 0)
    {
        return;
    }

    // connect target
    res = hscope_connect();
    // connect check
    if (res == 0)
    {
        return;
    }

    while (1)
    {
        // read data
        res = hscope_fetch_data();
        if (res == 0)
        {
            break;
        }
    }
}