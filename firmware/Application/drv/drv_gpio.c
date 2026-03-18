#include "drv_gpio.h"
#include "ch32v30x_rcc.h"

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
// | RSVD     | PB14     | N/A        | N/A        |           |
// | TMSO     | PB15     | JTAG_TMS   | SWDIO      | SPI2_MOSI |
// |          |          |            |            |           |
// -------------------------------------------------------------

/**
 * @brief 设置 TMS 引脚为 IO 输出模式
 *
 */
void drv_gpio_tms_io_out(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief 设置 TMS 引脚为 IO 输入模式
 *
 */
void drv_gpio_tms_io_in(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}


/**
 * @brief 初始化其他 GPIO 引脚
 *
 */
void drv_gpio_init_misc(void)
{
    // 使能 GPIOA, GPIOB, GPIOC 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    // ------------------------------------------------
    // | Pin name | Pin loc  | Function   | Pin alt   |
    // | -------- | -------- | ---------- | --------- |
    // | ADCV     | PA1      |            | ADC_CH1   |
    // | LED      | PC7      |            | GPIO      |
    // | SRST     | PC8      |            | GPIO      |
    // | 5VEN     | PC9      |            | GPIO      |
    // | SWDIR    | PB12     | SWDIR      | GPIO      |
    // ------------------------------------------------

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 配置 ADCV 为模拟输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置 LED, 5VEN, SRST 为推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 配置 SWDIR 为推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 RSVD 为浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 LED 为低电平
    drv_gpio_reset_led();
    // 配置 5VEN 为低电平
    drv_gpio_reset_5ven();
    // 配置 SWDIR 为高电平
    drv_gpio_set_swdir();
    // 配置 SRST 为高电平
    drv_gpio_set_srst();
}

/**
 * @brief 初始化 GPIO 为 JTAG 模式
 *
 */
void drv_gpio_init_as_jtag(void)
{
    // -------------------------------------------------
    // | Pin name | Pin loc  | Function   | Config     |
    // | -------- | -------- | ---------- | ---------- |
    // | TRST     | PC6      | JTAG_TRST  | Output     |
    // | TDI      | PB10     | JTAG_TDI   | Output     |
    // | TDO      | PB11     | JTAG_TDO   | Input      |
    // | TCK      | PB13     | JTAG_TCK   | Output     |
    // | TMS      | PB15     | JTAG_TMS   | Output     |
    // -------------------------------------------------

    // 配置 TRST 为推挽输出
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 配置 TDI, TDO, TCK, TMSO 为推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 TRST, SRST 为高电平
    drv_gpio_set_trst();
    drv_gpio_set_srst();
    // 配置 SWDIR 为高电平
    drv_gpio_set_swdir();
    // 配置 TDI, TCK, TMS 为高电平
    drv_gpio_set_tdi();
    drv_gpio_set_tck();
    drv_gpio_set_tms();
}

/**
 * @brief 初始化 GPIO 为 SWD + UART 模式
 *
 */
void drv_gpio_init_as_swd_uart(void)
{
    // -------------------------------------------------
    // | Pin name | Pin loc  | Function   | Config     |
    // | -------- | -------- | ---------- | ---------- |
    // | TRST     | PC6      | Reserved   | Input      |
    // | TDI      | PB10     | UART TXD   | Output     |
    // | TDO      | PB11     | UART RXD   | Input      |
    // | TCK      | PB13     | SWCLK      | Output     |
    // | TMS      | PB15     | SWDO       | Output     |
    // -------------------------------------------------

    // 配置 TRST 为浮空输入
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 配置 TDI 为复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 TDO 为复用浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 TCK 为复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 TMS 为推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    drv_gpio_set_srst();
    drv_gpio_set_tck();
    drv_gpio_set_swdir();
    drv_gpio_set_tms();
}

/**
 * @brief 初始化 GPIO 为 Hi-Z 模式
 *
 */
void drv_gpio_init_as_hiz(void)
{
    // -------------------------------------------------
    // | Pin name | Pin loc  | Function   | Config     |
    // | -------- | -------- | ---------- | ---------- |
    // | TRST     | PC6      | Reserved   | Input      |
    // | TDI      | PB10     | Reserved   | Input      |
    // | TDO      | PB11     | Reserved   | Input      |
    // | TCK      | PB13     | Reserved   | Input      |
    // | TMS      | PB15     | Reserved   | Input      |
    // -------------------------------------------------

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 配置 TRST, SRST 为浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 配置 TDI, TDO, TCK, TMS 为浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 SWDIR 为高电平
    drv_gpio_set_swdir();
    // 配置 SRST 为高电平
    drv_gpio_set_srst();
}