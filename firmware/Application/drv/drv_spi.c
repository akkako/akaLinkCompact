#include "drv_spi.h"

#include "ch32v30x_rcc.h"

extern void drv_spi_init (uint32_t speed) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
    SPI_Cmd (SPI1, DISABLE);

    RCC_APB2PeriphResetCmd (RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphResetCmd (RCC_APB2Periph_SPI1, DISABLE);

    SPI_InitTypeDef SPI_InitStructure = {0};
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    // if (speed >= 12000000) {
        // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    // } else if (speed >= 6000000) {
        // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    // } else if (speed >= 3000000) {
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    // } else if (speed >= 1500000) {
        // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    // } else if (speed >= 750000) {
    //     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    // } else if (speed >= 375000) {
    //     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    // } else {
    //     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    // }
    
    SPI_Init (SPI1, &SPI_InitStructure);
    SPI_Cmd (SPI1, ENABLE);
}
