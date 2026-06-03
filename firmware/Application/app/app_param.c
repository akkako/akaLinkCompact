#include "app_param.h"
#include <string.h>
#include <stdio.h>
#include "drv_gpio.h"
#include "ch32v30x_flash.h"
#include "drv_bkp.h"

#define APP_PARAM_ADDR_OFFSET ((uint32_t)0x0800F000)
#define FLASH_PAGE_SIZE (4096)

#define CMD_NOT_SUPPORT (0x00)
#define CMD_GET_CONFIG (0x01)
#define CMD_SET_CONFIG (0x02)
#define CMD_GET_VOLTAGE (0x03)
#define CMD_SAVE_CONFIG (0x04)
#define CMD_GET_MODEL (0x10)
#define CMD_GET_SERIAL (0x11)
#define CMD_RESET_DEVICE (0xFE)
#define CMD_ENTER_DFU (0xFF)

app_param_t g_param;

void app_param_load (void) {
    memcpy (&g_param, (const void *)(APP_PARAM_ADDR_OFFSET), sizeof (app_param_t));
    if (g_param.magic_number != 0x0D000721) {
        // default value
        printf ("Default value\r\n");
        g_param.magic_number = 0x0D000721;
        g_param.output_mode = 0;
        g_param.swd_sim_mode = 0;
        g_param.usb5v_out_mode = 0;
        g_param.clock_accel_mode = 0;
        app_param_save();
        printf ("Save default, size=%d\r\n", (int)sizeof(app_param_t));
    } else {
        if (g_param.usb5v_out_mode) {
            drv_gpio_set_5ven();
        } else {
            drv_gpio_reset_5ven();
        }
    }
}

void app_param_save (void) {
    FLASH_Status FLASHStatus = FLASH_COMPLETE;

    printf("Flash Save Start, CTLR=%08X\r\n", (unsigned)FLASH->CTLR);

    FLASH_Unlock();

    /* 配置 FLASH 访问时钟为系统时钟的一半 (144MHz/2 = 72MHz < 60MHz 限制)
     * 注意：必须在 FLASH_Unlock 之后调用 */
    FLASH_Access_Clock_Cfg(FLASH_Access_SYSTEM_HALF);

    printf("After clock cfg, CTLR=%08X\r\n", (unsigned)FLASH->CTLR);

    FLASH_ClearFlag (FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR);

    /* 检查 FLASH 状态 */
    FLASHStatus = FLASH_GetStatus();
    printf("Before erase, status=%d\r\n", FLASHStatus);

    FLASHStatus = FLASH_ErasePage (APP_PARAM_ADDR_OFFSET);  // Erase 4KB
    if (FLASHStatus != FLASH_COMPLETE) {
        printf("FLASH Erase Fail, status=%d\r\n", FLASHStatus);
        FLASH_Lock();
        return;
    }
    printf("Erase OK\r\n");

    uint16_t *p_data = (uint16_t *)&g_param;
    for (size_t addr = 0; addr < sizeof (app_param_t); addr += 2) {
        FLASHStatus = FLASH_ProgramHalfWord (APP_PARAM_ADDR_OFFSET + addr, p_data[addr / 2]);
        if (FLASHStatus != FLASH_COMPLETE) {
            printf("FLASH Program Fail at addr=%08X, status=%d, STATR=%08X\r\n",
                   APP_PARAM_ADDR_OFFSET + addr, FLASHStatus, (unsigned)FLASH->STATR);
            FLASH_Lock();
            return;
        }
    }

    FLASH_Lock();

    /* 验证写入的数据 */
    uint32_t *p_flash = (uint32_t *)APP_PARAM_ADDR_OFFSET;
    uint32_t *p_ram = (uint32_t *)&g_param;
    int verify_ok = 1;
    for (size_t i = 0; i < sizeof(app_param_t) / 4; i++) {
        if (p_flash[i] != p_ram[i]) {
            verify_ok = 0;
            printf("Verify Fail: offset=%d, flash=0x%08X, ram=0x%08X\r\n", (int)i*4, (unsigned)p_flash[i], (unsigned)p_ram[i]);
        }
    }

    if (verify_ok) {
        printf("Flash Save OK, size=%d\r\n", (int)sizeof(app_param_t));
    } else {
        printf("Flash Save Verify FAIL\r\n");
    }
}

void app_param_proc_hid (uint8_t *req_hid, uint8_t *res_hid) {
    uint8_t cmd = req_hid[2];
    switch (cmd) {
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
        printf ("Set Config %d %d %d %d\r\n", g_param.output_mode, g_param.swd_sim_mode, g_param.usb5v_out_mode, g_param.clock_accel_mode);
        break;
    case CMD_GET_VOLTAGE: {
        uint16_t vol = 3328;
        res_hid[1] = 3;
        res_hid[2] = CMD_GET_VOLTAGE;
        res_hid[3] = (vol >> 0) & 0xFF;
        res_hid[4] = (vol >> 8) & 0xFF;
    } break;
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
    case CMD_SAVE_CONFIG:
        app_param_save();
        res_hid[1] = 0x01;
        res_hid[2] = CMD_SAVE_CONFIG;
        printf ("Save Config\r\n");
        break;
    case CMD_RESET_DEVICE:
        printf ("Reset Device\r\n");
        NVIC_SystemReset();
        break;
    case CMD_ENTER_DFU:
        printf ("Enter DFU\r\n");
        drv_bkp_write_reg (0x0721);
        NVIC_SystemReset();
        break;
    default:
        printf ("Unsupport CMD\r\n");
        res_hid[1] = 0x01;
        res_hid[2] = CMD_NOT_SUPPORT;
        break;
    }
}