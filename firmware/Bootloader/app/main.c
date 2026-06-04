
#include "ch32v30x_misc.h"
#include "drv_gpio.h"
#include "drv_print.h"
#include "drv_systick.h"
#include "drv_bkp.h"
#include "flash_fat16.h"
#include "sys_define.h"
#include "ch32v30x_rcc.h"

uint8_t g_app_download_finished = 0;
uint8_t g_app_verify_state = 0;
uint8_t g_app_strap_state = 0;
uint32_t g_app_calc_crc32 = 0;
uint32_t g_app_file_crc32 = 0;
uint32_t g_app_length = 0;

extern void usb_msc_init(uint8_t busid, uintptr_t reg_base);

static inline uint32_t crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
    crc ^= 0xffffffff;
    while (len--)
    {
        crc ^= *buf++;
        for (uint8_t i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

static inline void get_app_info(void)
{
    g_app_length = *(uint32_t *)(APPLICATION_CODE_LENGTH_ADDR);
    g_app_file_crc32 = *(uint32_t *)(APPLICATION_CODE_CRC32_ADDR);
}

static inline void app_check(void)
{
    if (g_app_length > APPLICATION_MAX_SIZE)
    {
        return;
    }
    g_app_calc_crc32 = crc32(0x0D000721, (uint8_t *)APPLICATION_START_ADDR, g_app_length);

    if (g_app_calc_crc32 == g_app_file_crc32)
    {
        g_app_verify_state = 1;
    }
}

static inline void check_strap_state(void)
{
    uint16_t strap_data = drv_bkp_read_reg();
    if (strap_data != 0x0000)
    {
        drv_bkp_write_reg(0);
    }

    if (strap_data == 0x0721)
    {
        g_app_strap_state = 1;
    }
}

static inline void jump_to_app(void)
{
    NVIC_DisableIRQ(USBHS_IRQn);
    NVIC_DisableIRQ(USBFS_IRQn);

    NVIC_EnableIRQ(Software_IRQn);
    NVIC_SetPendingIRQ(Software_IRQn);
}

void SW_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SW_Handler(void) {
    __asm("li  a6, 0x7100");
    __asm("jr  a6");

    while(1);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    drv_print_init();
    drv_systick_init();
    drv_gpio_init_misc();
    drv_gpio_init_as_hiz();
    drv_bkp_init();

    get_app_info();
    app_check();
    check_strap_state();

    // 需要停在 Bootloader 的情况：
    // 1. app 校验失败
    // 2. 主动 starp

    if (g_app_verify_state != 1 || g_app_strap_state)
    {
        // 初始化 bootloader
        fat16_file_init();
        usb_msc_init(0, 0);
    }
    else
    {
        // 跳转应用程序
        jump_to_app();
    }

    while (1)
    {
        if (g_app_download_finished == 1)
        {
            drv_systick_delay_ms(1000);
            NVIC_SystemReset();
        }
    }
    return 0;
}
