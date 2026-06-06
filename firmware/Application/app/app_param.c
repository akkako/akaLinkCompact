#include "app_param.h"
#include <string.h>
#include <stdio.h>
#include "drv_gpio.h"
#include "ch32v30x_flash.h"
#include "drv_bkp.h"
#include "sys_config.h"

#define FLASH_PAGE_SIZE (256)
#define APP_PARAM_ADDR_OFFSET ((uint32_t)0x08000000 + 128 * 1024 - FLASH_PAGE_SIZE)

#define CMD_NOT_SUPPORT (0x00)
#define CMD_GET_CONFIG (0x01)
#define CMD_SET_CONFIG (0x02)
#define CMD_GET_VOLTAGE (0x03)
#define CMD_SAVE_CONFIG (0x04)
#define CMD_GET_MODEL (0x10)
#define CMD_GET_SERIAL (0x11)
#define CMD_GET_HWVER (0x12)
#define CMD_GET_FWVER (0x13)
#define CMD_GET_BLVER (0x14)
#define CMD_RESET_DEVICE (0xFE)
#define CMD_ENTER_DFU (0xFF)

app_param_t g_param;

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
        g_param.clock_accel_mode = 0;
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
    uint32_t buf[FLASH_PAGE_SIZE / 4] = {0xFFFFFFFF};

    memcpy(buf, &g_param, sizeof(app_param_t));

    FLASH_Unlock_Fast();
    FLASH_Access_Clock_Cfg(FLASH_Access_SYSTEM_HALF);

    FLASH_ErasePage_Fast(APP_PARAM_ADDR_OFFSET);
    FLASH_ProgramPage_Fast(APP_PARAM_ADDR_OFFSET, (uint32_t *)buf);

    FLASH_Lock_Fast();
}

void app_param_proc_hid(uint8_t *req_hid, uint8_t *res_hid)
{
    uint8_t cmd = req_hid[2];
    switch (cmd)
    {
    case CMD_GET_CONFIG:
        res_hid[1] = 0x05;
        res_hid[2] = CMD_GET_CONFIG;
        res_hid[3] = g_param.output_mode;
        res_hid[4] = g_param.swd_sim_mode;
        res_hid[5] = g_param.usb5v_out_mode;
        res_hid[6] = g_param.clock_accel_mode;
        break;
    case CMD_SET_CONFIG:
        g_param.output_mode = req_hid[3] ? 0x01 : 0x00;
        g_param.swd_sim_mode = req_hid[4] ? 0x01 : 0x00;
        g_param.usb5v_out_mode = req_hid[5] ? 0x01 : 0x00;
        g_param.clock_accel_mode = req_hid[6] ? 0x01 : 0x00;
        res_hid[1] = 1;
        res_hid[2] = CMD_SET_CONFIG;
        break;
    case CMD_GET_VOLTAGE:
    {
        uint16_t vol = 3328;
        res_hid[1] = 3;
        res_hid[2] = CMD_GET_VOLTAGE;
        res_hid[3] = (vol >> 0) & 0xFF;
        res_hid[4] = (vol >> 8) & 0xFF;
    }
    break;
    case CMD_GET_MODEL:
        res_hid[0x01] = 0x13;
        res_hid[0x02] = CMD_GET_MODEL;
        res_hid[0x03] = 'a';
        res_hid[0x04] = 'k';
        res_hid[0x05] = 'a';
        res_hid[0x06] = 'L';
        res_hid[0x07] = 'i';
        res_hid[0x08] = 'n';
        res_hid[0x09] = 'k';
        res_hid[0x0A] = ' ';
        res_hid[0x0B] = 'C';
        res_hid[0x0C] = 'M';
        res_hid[0x0D] = 'S';
        res_hid[0x0E] = 'I';
        res_hid[0x0F] = 'S';
        res_hid[0x10] = '-';
        res_hid[0x11] = 'D';
        res_hid[0x12] = 'A';
        res_hid[0x13] = 'P';
        res_hid[0x14] = 0x00;
        break;
    case CMD_GET_SERIAL:
        res_hid[0x01] = 0x0E;
        res_hid[0x02] = CMD_GET_SERIAL;
        res_hid[0x03] = '1';
        res_hid[0x04] = '2';
        res_hid[0x05] = '3';
        res_hid[0x06] = '4';
        res_hid[0x07] = '5';
        res_hid[0x08] = '6';
        res_hid[0x09] = '7';
        res_hid[0x0A] = '8';
        res_hid[0x0B] = '9';
        res_hid[0x0C] = '0';
        res_hid[0x0D] = 'A';
        res_hid[0x0E] = 'B';
        res_hid[0x0F] = 0x00;
        break;
    case CMD_GET_HWVER:
        res_hid[0x01] = 0x06;
        res_hid[0x02] = CMD_GET_HWVER;
        res_hid[0x03] = 'A';
        res_hid[0x04] = '.';
        res_hid[0x05] = '0';
        res_hid[0x06] = '1';
        res_hid[0x07] = 0x00;
        break;
    case CMD_GET_FWVER:
        res_hid[0x01] = 0x06;
        res_hid[0x02] = CMD_GET_FWVER;
        res_hid[0x03] = '1';
        res_hid[0x04] = '.';
        res_hid[0x05] = '1';
        res_hid[0x06] = '4';
        res_hid[0x07] = 0x00;
        break;
    case CMD_GET_BLVER:
        res_hid[0x01] = 0x06;
        res_hid[0x02] = CMD_GET_BLVER;
        res_hid[0x03] = '5';
        res_hid[0x04] = '.';
        res_hid[0x05] = '1';
        res_hid[0x06] = '4';
        res_hid[0x07] = 0x00;
        break;
    case CMD_SAVE_CONFIG:
        app_param_save();
        res_hid[1] = 0x01;
        res_hid[2] = CMD_SAVE_CONFIG;
        break;
    case CMD_RESET_DEVICE:
        NVIC_SystemReset();
        break;
    case CMD_ENTER_DFU:
        drv_bkp_write_reg(0x0721);
        NVIC_SystemReset();
        break;
    default:
        res_hid[1] = 0x01;
        res_hid[2] = CMD_NOT_SUPPORT;
        break;
    }
}