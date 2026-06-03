
#include "ch32v30x_misc.h"
#include "drv_gpio.h"
#include "drv_print.h"
#include "drv_systick.h"
#include "app_param.h"
#include "flash_fat16.h"

#define APP_COMPILE_TIME_STR_OFFSET (0)
#define APP_VERSION_STR_OFFSET (20)
#define APP_LENGTH_OFFSET (56)
#define APP_CRC32_OFFSET (60)
#define APP_FIRM_START_OFFSET (512)

uint8_t g_app_dfu_msc_state = 0;
uint8_t g_app_download_finished = 0;
uint8_t g_app_verify_state = 0;
uint8_t *g_app_compile_time = NULL;
uint8_t *g_app_version_str = NULL;
uint32_t g_app_calc_crc32 = 0;
uint32_t g_app_file_crc32 = 0;
uint32_t *g_app_length = NULL;

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
    g_app_compile_time = (uint8_t *)(VFAT16_FLASH_START_ADDR + APP_COMPILE_TIME_STR_OFFSET); // 编译时间字符串（20 字节）
    g_app_version_str = (uint8_t *)(VFAT16_FLASH_START_ADDR + APP_VERSION_STR_OFFSET);       // 版本字符串（36 字节）
    g_app_length = (uint32_t *)(VFAT16_FLASH_START_ADDR + APP_LENGTH_OFFSET);                // 应用程序长度（4 字节）
    g_app_file_crc32 = *(uint32_t *)(VFAT16_FLASH_START_ADDR + APP_CRC32_OFFSET);            // 文件 CRC32 校验值（4 字节）
}

static inline void app_check(void)
{
    if (*g_app_length > VFAT16_FLASH_SIZE - APP_FIRM_START_OFFSET)
    {
        return;
    }
    g_app_calc_crc32 = crc32(0x0D000721, (uint8_t *)(VFAT16_FLASH_START_ADDR + APP_FIRM_START_OFFSET), (*g_app_length));

    if (g_app_calc_crc32 == g_app_file_crc32)
    {
        g_app_verify_state = 1;
    }
}


int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);
    drv_print_init();
    drv_systick_init();
    app_param_load();
    drv_gpio_init_misc();
    drv_gpio_init_as_hiz();

    get_app_info();
    app_check();
	
    fat16_file_init();
    usb_msc_init(0, 0);

	while(1)
    {
        if (g_app_download_finished == 1)
        {
            drv_systick_delay_ms(1000);
            NVIC_SystemReset();
        }
	}
    return 0;
}
