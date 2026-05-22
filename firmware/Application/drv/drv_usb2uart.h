#ifndef __DRV_USB2UART_H__
#define __DRV_USB2UART_H__

#include <stdint.h>

#define USART_BUFFER_SIZE (4*1024)

extern volatile uint16_t Rx_RemainLen;
extern uint8_t usart_rx_buffer[USART_BUFFER_SIZE];

void drv_usb2uart_init(void);
void drv_usb2uart_gpio_af_uart(void);
void drv_usb2uart_tx_dma(uint8_t *data, uint16_t len);

#endif
