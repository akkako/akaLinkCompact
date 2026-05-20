#ifndef __DAP_PORT_H__
#define __DAP_PORT_H__

#include "DAP_config.h"

#include "ch32v30x_gpio.h"
#include "ch32v30x_rcc.h"
#include "drv_spi.h"

/**
 * @brief DAP 获取厂商字符串
 *
 * @param str 字符串指针
 * @return 字符串长度
 */
__STATIC_INLINE uint8_t DAP_GetVendorString (char *str) {
    str = DAP_STR_VENDOR_NAME;
    return sizeof (DAP_STR_VENDOR_NAME);
}

/**
 * @brief DAP 获取产品名称字符串
 *
 * @param str 字符串指针
 * @return 字符串长度
 */
__STATIC_INLINE uint8_t DAP_GetProductString (char *str) {
    str = DAP_STR_PRODUCT_NAME;
    return sizeof (DAP_STR_PRODUCT_NAME);
}

/**
 * @brief DAP 获取序列号字符串
 *
 * @param str 字符串指针
 * @return 字符串长度
 */
__STATIC_INLINE uint8_t DAP_GetSerNumString (char *str) {
    (void)str;
    return (0U);
}

/**
 * @brief DAP 获取固件版本字符串
 *
 * @param str 字符串指针
 * @return 字符串长度
 */
__STATIC_INLINE uint8_t DAP_GetProductFirmwareVersionString (char *str) {
    str = DAP_STR_FIRMWARE_VER;
    return sizeof (DAP_STR_FIRMWARE_VER);
}

// SWDIR -- PB12
// SWCK -- PB13
// SWDI -- PB14
// SWDO -- PB15

#define SWD_GPIO GPIOB
#define SWDIR_PIN GPIO_Pin_12
#define SWDI_PIN GPIO_Pin_14
#define SWDO_PIN GPIO_Pin_15
#define SWCK_PIN GPIO_Pin_13

#define SWCK_BIT_PIN (13 - 8)
#define SWDI_BIT_PIN (14 - 8)
#define SWDO_BIT_PIN (15 - 8)

/**
 * @brief DAP 引脚初始化
 * @note 初始化 GPIO 时钟
 *       初始化输入引脚为输入状态，输出引脚为高阻状态
 *       初始化复位输出引脚为上拉状态
 *       初始化 LED 状态
 *
 * @return None
 */
__STATIC_INLINE void DAP_SETUP (void) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init (GPIOC, &GPIO_InitStruct);
    GPIO_ResetBits (GPIOC, GPIO_Pin_8 | GPIO_Pin_9);
}

/**
 * @brief 获取当前时间戳
 *
 * @return 时间戳
 */
__STATIC_INLINE uint32_t TIMESTAMP_GET (void) {
    // return (DWT->CYCCNT);
    return 0;
}

/** DAP 引脚操作函数移植 **/

/** 初始化配置 JTAG 引脚，并配置默认输出电平
    TCK    --> 推挽输出/高电平
    TMS    --> 推挽输出/高电平
    TDI    --> 推挽输出/高电平
    TDO    --> 上拉输入
    nTRST  --> 推挽输出/高电平
    nRESET --> 开漏输出/高电平
*/
__STATIC_INLINE void PORT_JTAG_SETUP (void) {
    ;
}

/** 初始化配置 SWD 引脚，并配置默认输出电平
    TCK    --> 推挽输出/高电平
    TMS    --> 推挽输出/高电平
    TDI    --> 高阻
    TDO    --> 高阻
    nTRST  --> 高阻
    nRESET --> 开漏输出/高电平
*/
__STATIC_INLINE void PORT_SWD_SETUP (void) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_SetBits (SWD_GPIO, SWCK_PIN | SWDO_PIN);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = SWCK_PIN | SWDO_PIN | SWDIR_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (SWD_GPIO, &GPIO_InitStruct);
    GPIO_SetBits (SWD_GPIO, SWCK_PIN | SWDO_PIN | SWDIR_PIN);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Pin = SWDI_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (SWD_GPIO, &GPIO_InitStruct);

    drv_spi_init (4500000);
}

/** 禁用调试接口引脚输出
    TCK    --> 高阻
    TMS    --> 高阻
    TDI    --> 高阻
    TDO    --> 高阻
    nTRST  --> 高阻
    nRESET --> 高阻
*/
__STATIC_INLINE void PORT_OFF (void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Pin = SWCK_PIN | SWDO_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (SWD_GPIO, &GPIO_InitStruct);
    GPIO_ResetBits (SWD_GPIO, SWDIR_PIN);
}

// SWCLK/TCK 引脚 -------------------------------------

/**
 * @brief 读取 SWCLK/TCK 引脚当前输出电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_SWCLK_TCK_IN (void) {
    return GPIO_ReadOutputDataBit (SWD_GPIO, SWCK_PIN);
}

/**
 * @brief 设置 SWCLK/TCK 引脚输出电平为高
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_SET (void) {
    GPIO_SetBits (SWD_GPIO, SWCK_PIN);
}

/**
 * @brief 设置 SWCLK/TCK 引脚输出电平为低
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_CLR (void) {
    GPIO_ResetBits (SWD_GPIO, SWCK_PIN);
}

// SWDIO/TMS 引脚 --------------------------------------

/**
 * @brief 读取 SWDIO/TMS 引脚当前输入电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_TMS_IN (void) {
    return GPIO_ReadInputDataBit (SWD_GPIO, SWDI_PIN);
}

/**
 * @brief 设置 SWDIO/TMS 引脚输出电平为高
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_SET (void) {
    GPIO_SetBits (SWD_GPIO, SWDO_PIN);
}

/**
 * @brief 设置 SWDIO/TMS 引脚输出电平为低
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_CLR (void) {
    GPIO_ResetBits (SWD_GPIO, SWDO_PIN);
}

#include "drv_spi.h"

/**
 * @brief 配置 SWDIO 引脚为输出模式
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_ENABLE (void) {
    SWD_GPIO->CFGLR = (SWD_GPIO->CFGHR & ~PIN_CFG_CLR (SWDO_BIT_PIN)) | PIN_CFG_OUT (SWDO_BIT_PIN);
    GPIO_SetBits (SWD_GPIO, SWDIR_PIN);
}

/**
 * @brief 配置 SWDIO 引脚为输入模式
 *
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_DISABLE (void) {
    // SWD_GPIO->CFGLR = (SWD_GPIO->CFGHR & ~PIN_CFG_CLR(SWDO_PIN)) | PIN_CFG_IN(SWDO_PIN);
    GPIO_ResetBits (SWD_GPIO, SWDIR_PIN);
}

/**
 * @brief 读取 SWDIO 引脚当前输入电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN (void) {
    return PIN_SWDIO_TMS_IN();
}

/**
 * @brief 设置 SWDIO 引脚输出电平
 *
 * @param bit 最低位为输出电平
 * @return None
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT (uint32_t bit) {
    if (bit & 0x01) {
        PIN_SWDIO_TMS_SET();
    } else {
        PIN_SWDIO_TMS_CLR();
    }
}

// TDI 引脚 ---------------------------------------------

/**
 * @brief 读取 TDI 引脚输出电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_TDI_IN (void) {
    return (0U);
}

/**
 * @brief 设置 TDI 引脚输出电平
 *
 * @param bit 输出电平状态
 * @return None
 */
__STATIC_FORCEINLINE void PIN_TDI_OUT (uint32_t bit) {
    ;
}

// TDO 引脚 ---------------------------------------------

/**
 * @brief 读取 TDO 引脚输出电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_TDO_IN (void) {
    return (0U);
}

// nTRST 引脚 -------------------------------------------

/**
 * @brief 读取 nTRST 引脚输出电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_nTRST_IN (void) {
    return (0U);
}

/**
 * @brief 设置 nTRST 引脚输出电平
 *
 * @param bit 输出电平，0为复位状态，1为释放状态
 * @return None
 */
__STATIC_FORCEINLINE void PIN_nTRST_OUT (uint32_t bit) {
    ;
}

// nRESET 引脚 ------------------------------------------

/**
 * @brief 读取 nRESET 引脚输出电平
 *
 * @return 电平状态
 */
__STATIC_FORCEINLINE uint32_t PIN_nRESET_IN (void) {
    return (0U);
}

/**
 * @brief 设置 nRESET 引脚输出电平
 *
 * @param bit 输出电平，0为复位状态，1为释放状态
 * @return None
 */
__STATIC_FORCEINLINE void PIN_nRESET_OUT (uint32_t bit) {
    ;
}

// LED 引脚 ------------------------------------------

/**
 * @brief 连接指示 LED 状态设置
 *
 * @param bit LED 是否点亮
 * @return None
 */
__STATIC_INLINE void LED_CONNECTED_OUT (uint32_t bit) {
    if (bit) {
        GPIO_SetBits (GPIOC, GPIO_Pin_8);
    } else {
        GPIO_ResetBits (GPIOC, GPIO_Pin_8);
    }
}

/** Debug Unit: Set status Target Running LED.
\param bit status of the Target Running LED.
           - 1: Target Running LED ON: program execution in target started.
           - 0: Target Running LED OFF: program execution in target stopped.
*/

/**
 * @brief 运行指示 LED 状态设置
 *
 * @param bit LED 是否点亮
 * @return None
 */
__STATIC_INLINE void LED_RUNNING_OUT (uint32_t bit) {
    if (bit) {
        GPIO_SetBits (GPIOC, GPIO_Pin_9);
    } else {
        GPIO_ResetBits (GPIOC, GPIO_Pin_9);
    }
}

/**
 * @brief 通过特定方式复位目标板
 *
 * @return 0为未执行特定复位方式，1为执行完成特定复位动作
 */
__STATIC_INLINE uint8_t RESET_TARGET (void) {
    return (0U);
}


#endif
