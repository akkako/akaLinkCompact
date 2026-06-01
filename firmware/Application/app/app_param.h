#ifndef __APP_PARAM_H__
#define __APP_PARAM_H__

#include <stdint.h>

typedef struct _app_param_t {
    uint8_t output_mode;
    uint8_t swd_sim_mode;
    uint8_t usb5v_out_mode;
    uint8_t overclock_mode;
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
