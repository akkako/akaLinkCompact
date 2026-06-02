#include "app_param.h"
#include <string.h>
#include "drv_gpio.h"
#include "ch32v30x_flash.h"
#include "drv_bkp.h"

#define APP_PARAM_ADDR_OFFSET ((uint32_t)0x08000000 + (124 * 1024))
#define FLASH_PAGE_SIZE (4096)

#define CMD_OUTPUT_MODE_SET (0x01)
#define CMD_OUTPUT_MODE_GET (0x81)
#define CMD_SWD_SIM_MODE_SET (0x02)
#define CMD_SWD_SIM_MODE_GET (0x82)
#define CMD_5V_OUT_MODE_SET (0x03)
#define CMD_5V_OUT_MODE_GET (0x83)

#define CMD_TARGET_VOLTAGE_GET (0x84)
#define CMD_PARAM_SAVE_SET (0x05)
#define CMD_ENTER_DFU_SET (0x06)

app_param_t g_param;

uint32_t EraseCounter = 0x0, Address = 0x0;
uint16_t Data = 0xAAAA;
uint32_t WRPR_Value = 0xFFFFFFFF, ProtectedPages = 0x0;
uint32_t NbrOfPage;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

void app_param_load(void)
{
    memcpy(&g_param, (const void *)(APP_PARAM_ADDR_OFFSET), sizeof(app_param_t));
    if (g_param.magic_number != 0x0D000721)
    {
        // default value
        g_param.magic_number = 0x0D000721;
        g_param.output_mode = 0;
        g_param.swd_sim_mode = 0;
        g_param.usb5v_out_mode = 0;
        g_param.padding = 0;
        app_param_save();
    }
    else
    {
        if (g_param.usb5v_out_mode)
        {
            drv_gpio_set_5ven();
        }
        else
        {
            drv_gpio_reset_5ven();
        }
    }
}

void app_param_save(void)
{
    FLASH_Unlock();

    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR);
    FLASHStatus = FLASH_ErasePage(APP_PARAM_ADDR_OFFSET); // Erase 4KB
    if (FLASHStatus != FLASH_COMPLETE)
    {
        // printf("FLASH Erase Fail\r\n");
        return;
    }
    // printf("FLASH Erase Suc\r\n");
    uint16_t *p_data = (uint16_t *)&g_param;
    for (size_t addr = 0; addr < sizeof(app_param_t); addr += 2)
    {
        FLASHStatus = FLASH_ProgramHalfWord(APP_PARAM_ADDR_OFFSET + addr, p_data[addr / 2]);
        if (FLASHStatus != FLASH_COMPLETE)
        {
            return;
        }
    }
    FLASH_Lock();
}

void app_param_proc_hid(uint8_t *req_hid, uint8_t *res_hid)
{
    uint8_t cmd = req_hid[2];
    switch (cmd)
    {
    case CMD_OUTPUT_MODE_SET:
        g_param.output_mode = req_hid[3] ? 0x01 : 0x00;
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.output_mode;
        break;
    case CMD_OUTPUT_MODE_GET:
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.output_mode;
        break;
    case CMD_SWD_SIM_MODE_SET:
        g_param.swd_sim_mode = req_hid[3] ? 0x01 : 0x00;
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.swd_sim_mode;
        break;
    case CMD_SWD_SIM_MODE_GET:
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.swd_sim_mode;
        break;
    case CMD_5V_OUT_MODE_SET:
        g_param.usb5v_out_mode = req_hid[3] ? 0x01 : 0x00;
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.usb5v_out_mode;
        break;
    case CMD_5V_OUT_MODE_GET:
        res_hid[1] = 2;
        res_hid[2] = cmd;
        res_hid[3] = g_param.usb5v_out_mode;
        break;
    case CMD_TARGET_VOLTAGE_GET:
        res_hid[1] = 3;
        res_hid[2] = cmd;
        res_hid[3] = 0x00;
        res_hid[4] = 0x00;
        break;
    case CMD_PARAM_SAVE_SET:
        app_param_save();
        NVIC_SystemReset();
        break;
    case CMD_ENTER_DFU_SET:
        drv_bkp_write_reg(0x0721);
        NVIC_SystemReset();
        break;
    default:
        res_hid[1] = 1;
        res_hid[2] = 0x00;
        break;
    }
}