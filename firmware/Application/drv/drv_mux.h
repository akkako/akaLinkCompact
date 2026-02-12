#ifndef __DRV_MUX_H__
#define __DRV_MUX_H__

void drv_mux_swd_init(void);
void drv_mux_swd_spi(void);
void drv_mux_swd_gpio(void);

void drv_mux_jtag_init(void);
void drv_mux_jtag_spi(void);
void drv_mux_jtag_gpio(void);

#endif
