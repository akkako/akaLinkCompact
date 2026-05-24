#ifndef __DRV_USB2UART_H__
#define __DRV_USB2UART_H__

#include <stdint.h>

void drv_usb2uart_preinit (void);
void drv_usb2uart_gpio_af_uart (void);
void drv_usb2uart_enable (uint8_t enable);
void drv_usb2uart_set_linecoding (uint32_t dwDTERate, uint8_t bCharFormat, uint8_t bParityType, uint8_t bDataBits);
void drv_usb2uart_start_tx_dma (uint8_t *data, uint16_t len);
void drv_usb2uart_start_rx_dma (uint8_t *data, uint16_t len);

void drv_usb2uart_stop_tx_dma (void);
void drv_usb2uart_stop_rx_dma (void);

#endif
