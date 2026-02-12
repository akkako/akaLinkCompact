#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include "ch32v30x_conf.h"

extern void drv_spi_init (uint32_t speed);

/**
 * @brief 切换引脚复用为 SPI
 * 
 */
static inline void drv_spi_gpio_mux_spi() {
    uint32_t temp = GPIOA->CFGLR;
    temp &= ~(uint32_t)(0xF << (4 * 7) | 0xF << (4 * 5));
    temp |= (uint32_t)(0xB << (4 * 7) | 0xB << (4 * 5));
    GPIOA->CFGLR = temp;
}

/**
 * @brief 切换引脚复用为 GPIO
 * 
 */
static inline void drv_spi_gpio_mux_gpio() {
    uint32_t temp = GPIOA->CFGLR;
    temp &= ~(uint32_t)(0xF << (4 * 7) | 0xF << (4 * 5));
    temp |= (uint32_t)(0x3 << (4 * 7) | 0x3 << (4 * 5));
    GPIOA->CFGLR = temp;
}


static inline void drv_spi_tx_start()
{
    SPI1->CTLR1 |= SPI_CTLR1_BIDIOE | SPI_CTLR1_SPE; // 发送模式,SPI 使能
}

static inline void drv_spi_tx (uint8_t data) {
    SPI1->DATAR = data;
    // while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
}

static inline void drv_spi_tx_wait()
{
    while ((SPI1->STATR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
}

static inline void drv_spi_tx_once (uint8_t data) {
    SPI1->DATAR = data;
    while ((SPI1->STATR & SPI_I2S_FLAG_BSY) != (uint16_t)RESET);
    SPI1->CTLR1 &= ~SPI_CTLR1_SPE;
}


static inline void drv_spi_tx_end()
{
    while ((SPI1->STATR & SPI_I2S_FLAG_BSY) != (uint16_t)RESET);
    SPI1->CTLR1 &= ~SPI_CTLR1_SPE;
}

static inline void drv_spi_rx_start()
{
    SPI1->CTLR1 &= ~SPI_CTLR1_BIDIOE; // 接收模式
    SPI1->CTLR1 |= SPI_CTLR1_SPE; // SPI 使能
}

static inline uint8_t drv_spi_rx()
{
    while((SPI1->STATR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
    return SPI1->DATAR;
}

static inline uint8_t drv_spi_rx_end()
{
    while((SPI1->STATR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
    SPI1->CTLR1 &= ~SPI_CTLR1_SPE;
    return SPI1->DATAR;
}

#endif
