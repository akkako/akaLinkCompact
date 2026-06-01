#include "app_param.h"
#include <string.h>
#include "drv_gpio.h"

#define APP_PARAM_ADDR_OFFSET (0x00000000)

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

void app_param_load (void) {
    memcpy (&g_param, (const void *)(APP_PARAM_ADDR_OFFSET), sizeof (app_param_t));
    if (g_param.magic_number != 0x0D000721) {
        // default value
        g_param.magic_number = 0x0D000721;
        g_param.output_mode = 0;
        g_param.swd_sim_mode = 0;
        g_param.usb5v_out_mode = 0;
        g_param.padding = 0;
        app_param_save();
    } else {
        if (g_param.usb5v_out_mode) {
            drv_gpio_set_5ven();
        } else {
            drv_gpio_reset_5ven();
        }
    }
}

void app_param_save (void) {
}

void app_param_proc_hid (uint8_t *req_hid, uint8_t *res_hid) {
}