#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include "ch32v30x.h"
#include "ch32v30x_spi.h"
#include "ch32v30x_dma.h"

#define CTLR1_SPE_Set ((uint16_t)0x0040)
#define CTLR1_SPE_Reset ((uint16_t)0xFFBF)

/*
| Name     | Pin      | JTAG       | SWD        | Alternate |
| -------- | -------- | ---------- | ---------- | --------- |
| TDI      | PB10     | JTAG_TDI   | UART_TXD   | UART3_TXD |
| TDO      | PB11     | JTAG_TDO   | UART_RXD   | UART3_RXD |
| SWDIR    | PB12     | N/A(HIGH)  | SWDIR      | GPIO      |
| TCK      | PB13     | JTAG_TCK   | SWCLK      | SPI2_SCLK |
| TMSI     | PB14     | N/A(INPUT) | SWDI       | SPI2_MISO |
| TMSO     | PB15     | JTAG_TMS   | SWDO       | SPI2_MOSI |
*/

#define TDI_BIT_PIN (10 - 8)
#define TDO_BIT_PIN (11 - 8)
#define SWDIR_BIT_PIN (12 - 8)
#define TCK_BIT_PIN (13 - 8)
#define TMSI_BIT_PIN (14 - 8)
#define TMSO_BIT_PIN (15 - 8)

#define IO_CFG_IN_ANALOG(x) (0x0 << (4 * x))
#define IO_CFG_IN_FLOAT(x) (0x4 << (4 * x))
#define IO_CFG_IN_PULL(x) (0x8 << (4 * x))
#define IO_CFG_OUT_PP(x) (0x3 << (4 * x))
#define IO_CFG_OUT_OD(x) (0x7 << (4 * x))
#define IO_CFG_OUT_AFPP(x) (0xB << (4 * x))
#define IO_CFG_OUT_AFOD(x) (0xF << (4 * x))


uint32_t drv_spi_get_speed (void);
extern void drv_spi_init (uint32_t speed);
void drv_spi_swd_init (uint32_t speed);
void drv_spi_jtag_init (uint32_t speed);
void drv_spi_flash_init (uint32_t speed);

/**
 * @brief 切换引脚复用为 SPI
 *
 */
static inline void drv_spi_gpio_mux_spi() {
    // register uint32_t temp = GPIOB->CFGHR;
    // temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    // temp |= (uint32_t)(PIN_CFG_AF (SWDO_BIT_PIN) | PIN_CFG_AF (SWCK_BIT_PIN));
    // GPIOB->CFGHR = temp;
    GPIOB->CFGHR = 0x44 |
                   IO_CFG_OUT_AFPP (TDI_BIT_PIN) |
                   IO_CFG_IN_PULL (TDO_BIT_PIN) |
                   IO_CFG_OUT_PP (SWDIR_BIT_PIN) |
                   IO_CFG_OUT_AFPP (TCK_BIT_PIN) |
                   IO_CFG_IN_PULL (TMSI_BIT_PIN) |
                   IO_CFG_OUT_AFPP (TMSO_BIT_PIN);
}

/**
 * @brief 切换引脚复用为 GPIO 输出
 *
 */
static inline void drv_spi_gpio_mux_gpio_out() {
    // register uint32_t temp = GPIOB->CFGHR;
    // temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    // temp |= (uint32_t)(PIN_CFG_OUT (SWDO_BIT_PIN) | PIN_CFG_OUT (SWCK_BIT_PIN));
    // GPIOB->CFGHR = temp;
    GPIOB->CFGHR = 0x44 |
                   IO_CFG_OUT_AFPP (TDI_BIT_PIN) |
                   IO_CFG_IN_PULL (TDO_BIT_PIN) |
                   IO_CFG_OUT_PP (SWDIR_BIT_PIN) |
                   IO_CFG_OUT_PP (TCK_BIT_PIN) |
                   IO_CFG_IN_PULL (TMSI_BIT_PIN) |
                   IO_CFG_OUT_PP (TMSO_BIT_PIN);
}

/**
 * @brief 切换引脚复用为 GPIO 输入
 *
 */
static inline void drv_spi_gpio_mux_gpio_in() {
    // register uint32_t temp = GPIOB->CFGHR;
    // temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    // temp |= (uint32_t)(PIN_CFG_IN (SWDO_BIT_PIN) | PIN_CFG_IN (SWCK_BIT_PIN));
    // GPIOB->CFGHR = temp;
    GPIOB->CFGHR = 0x44 |
                   IO_CFG_OUT_AFPP (TDI_BIT_PIN) |
                   IO_CFG_IN_PULL (TDO_BIT_PIN) |
                   IO_CFG_OUT_PP (SWDIR_BIT_PIN) |
                   IO_CFG_OUT_PP (TCK_BIT_PIN) |
                   IO_CFG_IN_PULL (TMSI_BIT_PIN) |
                   IO_CFG_IN_FLOAT (TMSO_BIT_PIN);
}

static inline void drv_spi_dma_wait_rx (void) {
    while ((!DMA_GetFlagStatus (DMA1_FLAG_TC4)) || (!DMA_GetFlagStatus (DMA1_FLAG_TC5)));
    while ((SPI2->STATR & SPI_I2S_FLAG_BSY));
    DMA1_Channel4->CFGR = 0;
    DMA1_Channel5->CFGR = 0;
}

static inline void drv_spi_dma_wait_tx (void) {
    while (!DMA_GetFlagStatus (DMA1_FLAG_TC5));
    while ((SPI2->STATR & SPI_I2S_FLAG_BSY));
    DMA1_Channel5->CFGR = 0;
}

static inline void drv_spi_dma_rx_preset (uint8_t *txd, uint8_t *rxd, uint16_t cnt) {
    DMA1_Channel4->CNTR = cnt;
    DMA1_Channel5->CNTR = cnt;
    DMA1_Channel4->MADDR = (uint32_t)rxd;
    DMA1_Channel5->MADDR = (uint32_t)txd;
}

static inline void drv_spi_dma_rx_start (void) {
    DMA1_Channel4->CFGR =
        DMA_CFGR1_EN |
        DMA_CFGR1_MINC |
        DMA_CFGR1_PL_1;

    DMA1_Channel5->CFGR =
        DMA_CFGR1_EN |
        DMA_CFGR1_DIR |
        // DMA_CFGR1_MINC |
        DMA_CFGR1_PL_1;
}

static inline void drv_spi_dma_tx_preset (uint8_t *txd, uint16_t cnt) {
    DMA1_Channel5->CNTR = cnt;
    DMA1_Channel5->MADDR = (uint32_t)txd;
}

static inline void drv_spi_dma_tx_start (void) {
    DMA1_Channel5->CFGR =
        DMA_CFGR1_EN |
        DMA_CFGR1_DIR |
        DMA_CFGR1_MINC |
        DMA_CFGR1_PL_1;
}

static inline void drv_spi_tx (uint8_t data) {
    SPI2->DATAR = data;
}

static inline uint8_t drv_spi_tx_wait() {
    while ((SPI2->STATR & SPI_I2S_FLAG_BSY));
    return SPI2->DATAR;
}

#endif
