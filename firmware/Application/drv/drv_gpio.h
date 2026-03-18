#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <stdint.h>
#include "ch32v30x_gpio.h"

// -------------------------------------------------------------
// | Pin name | Pin loc  | funtion1   | funtion2   | Pin alt   |
// | -------- | -------- | ---------- | ---------- | --------- |
// | ADCV     | PA1      |            |            | ADC_CH1   |
// | TRST     | PC6      | JTAG_TRST  | N/A(INPUT) | GPIO      |
// | LED      | PC7      |            |            | GPIO      |
// | SRST     | PC8      |            |            | GPIO      |
// | 5VEN     | PC9      |            |            | GPIO      |
// | TDI      | PB10     | JTAG_TDI   | UART_TXD   | UART3_TXD |
// | TDO      | PB11     | JTAG_TDO   | UART_RXD   | UART3_RXD |
// | SWDIR    | PB12     | N/A(HIGH)  | SWDIR      | GPIO      |
// | TCK      | PB13     | JTAG_TCK   | SWCLK      | SPI2_SCLK |
// | TMSI     | PB14     | N/A(INPUT) | SWDI       | SPI2_MISO |
// | TMSO     | PB15     | JTAG_TMS   | SWDO       | SPI2_MOSI |
// |          |          |            |            |           |
// -------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 设置 TRST 引脚为高电平
     *
     */
    static inline void drv_gpio_set_trst(void)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
    }

    /**
     * @brief 设置 TRST 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_trst(void)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
    }

    /**
     * @brief 设置 SRST 引脚为高电平（驱动 MOS 反相）
     *
     */
    static inline void drv_gpio_set_srst(void)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    }

    /**
     * @brief 设置 SRST 引脚为低电平（驱动 MOS 反相）
     *
     */
    static inline void drv_gpio_reset_srst(void)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_8);
    }

    /**
     * @brief 设置 5VEN 引脚为高电平
     *
     */
    static inline void drv_gpio_set_5ven(void)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_9);
    }

    /**
     * @brief 设置 5VEN 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_5ven(void)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    }

    /**
     * @brief 设置 LED 引脚为高电平
     *
     */
    static inline void drv_gpio_set_led(void)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_7);
    }

    /**
     * @brief 设置 LED 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_led(void)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
    }

    /**
     * @brief 设置 SWDIR 引脚为高电平
     *
     */
    static inline void drv_gpio_set_swdir(void)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    }

    /**
     * @brief 设置 SWDIR 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_swdir(void)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    }

    /**
     * @brief 设置 TDI 引脚为高电平
     *
     */
    static inline void drv_gpio_set_tdi(void)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_10);
    }

    /**
     * @brief 设置 TDI 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_tdi(void)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_10);
    }

    /**
     * @brief 设置 TCK 引脚为高电平
     *
     */
    static inline void drv_gpio_set_tck(void)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
    }

    /**
     * @brief 设置 TCK 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_tck(void)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }

    /**
     * @brief 设置 TMSO 引脚为高电平
     *
     */
    static inline void drv_gpio_set_tmso(void)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
    }

    /**
     * @brief 设置 TMSO 引脚为低电平
     *
     */
    static inline void drv_gpio_reset_tmso(void)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }

    void drv_gpio_init_as_hiz(void);
    void drv_gpio_init_as_jtag(void);
    void drv_gpio_init_as_swd_uart(void);

#ifdef __cplusplus
}
#endif

#endif
