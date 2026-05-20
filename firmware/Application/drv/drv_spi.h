#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include "ch32v30x.h"
#include "ch32v30x_spi.h"
#include "ch32v30x_dma.h"

#define CTLR1_SPE_Set ((uint16_t)0x0040)
#define CTLR1_SPE_Reset ((uint16_t)0xFFBF)

// SWDIR -- PB12
// SWCK -- PB13
// SWDI -- PB14
// SWDO -- PB15

#define SWCK_BIT_PIN (13 - 8)
#define SWDI_BIT_PIN (14 - 8)
#define SWDO_BIT_PIN (15 - 8)

#define PIN_CFG_CLR(x) (0xF << (4 * x))
#define PIN_CFG_IN(x) (0x1 << (4 * x))
#define PIN_CFG_OUT(x) (0x3 << (4 * x))
#define PIN_CFG_AF(x) (0xB << (4 * x))


uint32_t drv_spi_get_speed(void);
extern void drv_spi_init (uint32_t speed);
void drv_spi_swd_init (uint32_t speed);
void drv_spi_jtag_init (uint32_t speed);
void drv_spi_flash_init (uint32_t speed);

/**
 * @brief 切换引脚复用为 SPI
 *
 */
static inline void drv_spi_gpio_mux_spi() {
    register uint32_t temp = GPIOB->CFGHR;
    temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    temp |= (uint32_t)(PIN_CFG_AF (SWDO_BIT_PIN) | PIN_CFG_AF (SWCK_BIT_PIN));
    GPIOB->CFGHR = temp;
}

/**
 * @brief 切换引脚复用为 GPIO 输出
 *
 */
static inline void drv_spi_gpio_mux_gpio_out() {
    register uint32_t temp = GPIOB->CFGHR;
    temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    temp |= (uint32_t)(PIN_CFG_OUT (SWDO_BIT_PIN) | PIN_CFG_OUT (SWCK_BIT_PIN));
    GPIOB->CFGHR = temp;
}

/**
 * @brief 切换引脚复用为 GPIO 输入
 *
 */
static inline void drv_spi_gpio_mux_gpio_in() {
    register uint32_t temp = GPIOB->CFGHR;
    temp &= ~(uint32_t)(PIN_CFG_CLR (SWDO_BIT_PIN) | PIN_CFG_CLR (SWCK_BIT_PIN));
    temp |= (uint32_t)(PIN_CFG_IN (SWDO_BIT_PIN) | PIN_CFG_IN (SWCK_BIT_PIN));
    GPIOB->CFGHR = temp;
}

static inline void drv_spi_dma_wait(void) {
    while ((!DMA_GetFlagStatus (DMA1_FLAG_TC4)) || (!DMA_GetFlagStatus (DMA1_FLAG_TC5)));
    while ((SPI2->STATR & SPI_I2S_FLAG_BSY));
    DMA1_Channel4->CFGR = 0;
    DMA1_Channel5->CFGR = 0;
}

static inline void drv_spi_dma_transmit (uint8_t *txd, uint8_t *rxd, uint16_t cnt) {

    DMA1_Channel4->CNTR = cnt;
    DMA1_Channel5->CNTR = cnt;
    // DMA1_Channel4->PADDR = (uint32_t) & (SPI2->DATAR);
    // DMA1_Channel5->PADDR = (uint32_t) & (SPI2->DATAR);
    DMA1_Channel4->MADDR = (uint32_t)rxd;
    DMA1_Channel5->MADDR = (uint32_t)txd;

    DMA1_Channel4->CFGR =
        DMA_CFGR1_EN |
        DMA_CFGR1_MINC |
        DMA_CFGR1_PL_1;

    DMA1_Channel5->CFGR =
        DMA_CFGR1_EN |
        DMA_CFGR1_DIR |
        DMA_CFGR1_MINC |
        DMA_CFGR1_PL_1;
}

static inline uint8_t drv_spi_tx (uint8_t data) {
    SPI2->DATAR = data;
    // while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_TXE) == RESET);
    // while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_BSY) == SET);
    while ((SPI2->STATR & SPI_I2S_FLAG_BSY));

    return SPI2->DATAR;
}



#endif
