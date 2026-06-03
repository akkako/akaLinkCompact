#ifndef __APP_PARAM_H__
#define __APP_PARAM_H__

#include <stdint.h>

typedef struct _app_param_t {
    uint32_t magic_number;   // 放在前面，确保 4 字节对齐
    uint8_t output_mode;     // 0 - SWD+VCOM, 1 - SWD+JTAG
    uint8_t swd_sim_mode;    // 0 - SPI, 1 - GPIO
    uint8_t usb5v_out_mode;  // 0 - Disable, 1 - Enable
    uint8_t clock_accel_mode;// 0 - Disable, 1 - Enable
} app_param_t;

extern app_param_t g_param;

#ifdef __cplusplus
extern "C" {
#endif

extern void app_param_load (void);
extern void app_param_save (void);
extern void app_param_proc_hid (uint8_t *req_hid, uint8_t *res_hid);

#ifdef __cplusplus
}
#endif

#endif
