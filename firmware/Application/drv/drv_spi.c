#include "drv_spi.h"
#include "drv_clock.h"
#include "ch32v30x_rcc.h"
#include "ch32v30x_dma.h"

uint32_t spi_speed = 0;

extern void drv_spi_init (uint32_t speed) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit (DMA1_Channel4);
    DMA_DeInit (DMA1_Channel5);

    SPI_Cmd (SPI2, DISABLE);

    RCC_APB1PeriphResetCmd (RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphResetCmd (RCC_APB1Periph_SPI2, DISABLE);

    SPI_InitTypeDef SPI_InitStructure = {0};
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    if (speed >= SystemCoreClock / 4) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
        spi_speed = SystemCoreClock / 4;
    } else if (speed >= SystemCoreClock / 8) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
        spi_speed = SystemCoreClock / 8;
    } else if (speed >= SystemCoreClock / 16) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
        spi_speed = SystemCoreClock / 16;
    } else if (speed >= SystemCoreClock / 32) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
        spi_speed = SystemCoreClock / 32;
    } else if (speed >= SystemCoreClock / 64) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
        spi_speed = SystemCoreClock / 64;
    } else if (speed >= SystemCoreClock / 128) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
        spi_speed = SystemCoreClock / 128;
    } else if (speed >= SystemCoreClock / 256) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
        spi_speed = SystemCoreClock / 256;
    } else {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
        spi_speed = SystemCoreClock / 512;
    }

    SPI_Init (SPI2, &SPI_InitStructure);
    SPI_I2S_DMACmd (SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_I2S_DMACmd (SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_Cmd (SPI2, ENABLE);

    DMA1_Channel4->PADDR = (uint32_t)&(SPI2->DATAR);
    DMA1_Channel5->PADDR = (uint32_t)&(SPI2->DATAR);
}

uint32_t drv_spi_get_speed (void) {
    return spi_speed;
}