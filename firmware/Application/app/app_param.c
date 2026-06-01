#include "app_param.h"

#define APP_PARAM_ADDR_OFFSET (0x00000000)

#define CMD_OUTPUT_MODE_SET (0x01)
#define CMD_OUTPUT_MODE_GET (0x81)
#define CMD_SWD_SIM_MODE_SET (0x02)
#define CMD_SWD_SIM_MODE_GET (0x82)
#define CMD_5V_OUT_MODE_SET (0x03)
#define CMD_5V_OUT_MODE_GET (0x83)
#define CMD_OVERCLOCK_MODE_SET (0x04)
#define CMD_OVERCLOCK_MODE_GET (0x84)

#define CMD_TARGET_VOLTAGE_GET (0x85)
#define CMD_PARAM_SAVE_SET (0x06)
#define CMD_ENTER_DFU_SET (0x07)

app_param_t g_param;

void app_param_load (void) {
}

void app_param_save (void) {
}

void app_param_proc_hid (uint8_t *req_hid, uint8_t *res_hid) {

}