#include "drv_usb2uart.h"

#include "ch32v30x_dma.h"
#include "ch32v30x_rcc.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_usart.h"
#include "ch32v30x_misc.h"

#include "usb_main.h"
#include "drv_print.h"

// PB10  UART3_TX DMA_CH2
// PB11  UART3_RX DMA_CH3

#define DMA_CH_UART_TX DMA1_Channel2
#define DMA_CH_UART_RX DMA1_Channel3

uint8_t usart_rx_buffer[USART_BUFFER_SIZE];
volatile uint32_t g_uart_tx_transfer_length = 0;

static volatile uint32_t UARTx_Rx_DMACurCount;  /* Serial port receive dma current counter */
static volatile uint32_t UARTx_Rx_DMALastCount; /* Serial port receive dma last value counter  */

volatile uint16_t Rx_RemainLen; /* Remaining unprocessed length of the serial data receive buffer */

void DMA1_Channel2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)__attribute__((interrupt("WCH-Interrupt-fast")));


void DMA1_Channel2_IRQHandler(void)
{
    // 全满
    if (DMA1->INTFR & DMA1_IT_TC2) {
        DMA1->INTFCR = DMA1_IT_TC2;
        DMA_Cmd(DMA_CH_UART_TX, DISABLE); /* USART3 Tx */
        chry_dap_usb2uart_uart_send_complete(g_uart_tx_transfer_length);
    }
}

void DMA1_Channel3_IRQHandler(void)
{
    // 全满
    if (DMA1->INTFR & DMA1_IT_TC3) {
        DMA1->INTFCR = DMA1_IT_TC3;
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_IDLE))
    {
        
    }
}

void drv_usb2uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    USART_InitTypeDef USART_InitStructure = { 0 };
    NVIC_InitTypeDef NVIC_InitStructure = { 0 };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* USART3 TX--> PB10   RX -->P B11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

    DMA_InitTypeDef DMA_InitStructure = { 0 };
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // send dma
    DMA_DeInit(DMA_CH_UART_TX);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DATAR); /* USART2->DATAR:0x40004404 */
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CH_UART_TX, &DMA_InitStructure);

    // send complete irq setting
    DMA_ITConfig(DMA_CH_UART_TX, DMA_IT_TC, ENABLE); // 传输完成
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // recv dma
    DMA_DeInit(DMA_CH_UART_RX);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)usart_rx_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 512;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_Init(DMA_CH_UART_RX, &DMA_InitStructure);
    DMA_Cmd(DMA_CH_UART_RX, ENABLE); /* USART3 Rx */

    USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
}


void chry_dap_usb2uart_uart_config_callback(struct cdc_line_coding *line_coding)
{
    USART_InitTypeDef USART_InitStructure = { 0 };
    DMA_InitTypeDef DMA_InitStructure = { 0 };


    if(line_coding->dwDTERate > 9000000)
    {
        line_coding->dwDTERate = 9000000;
    }

    USART_DeInit(USART3);
    USART_InitStructure.USART_BaudRate = line_coding->dwDTERate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /* Number of stop bits (0: 1 stop bit; 1: 1.5 stop bits; 2: 2 stop bits). */
    if (line_coding->bCharFormat == 1) {
        USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
    } else if (line_coding->bCharFormat == 2) {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    } else {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    /* Check digit (0: None; 1: Odd; 2: Even; 3: Mark; 4: Space); */
    if (line_coding->bParityType == 1) {
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    } else if (line_coding->bParityType == 2) {
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    } else {
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);

    UARTx_Rx_DMACurCount = 0;
    UARTx_Rx_DMALastCount = 0;
    Rx_RemainLen = 0;
    DMA_CH_UART_RX->CFGR &= (uint16_t)(~DMA_CFGR1_EN);
    DMA_CH_UART_RX->CNTR = USART_BUFFER_SIZE;
    DMA_CH_UART_RX->CFGR |= DMA_CFGR1_EN;

    // send dma
    DMA_DeInit(DMA_CH_UART_TX);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DATAR); /* USART2->DATAR:0x40004404 */
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CH_UART_TX, &DMA_InitStructure);
    DMA_ITConfig(DMA_CH_UART_TX, DMA_IT_TC, ENABLE);

    // recv dma
    DMA_DeInit(DMA_CH_UART_RX);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)usart_rx_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 512;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_Init(DMA_CH_UART_RX, &DMA_InitStructure);
    // DMA_ITConfig(DMA_CH_UART_RX, DMA_IT_TC, ENABLE);

    DMA_Cmd(DMA_CH_UART_RX, ENABLE); /* USART3 Rx */

    USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
}

void drv_usb2uart_restart_rxdma(void)
{

}

void chry_dap_usb2uart_uart_send_bydma(uint8_t *data, uint16_t len)
{
    if (len <= 0) {
        return;
    }
    g_uart_tx_transfer_length = len;
    // Start DMA
    DMA_CH_UART_TX->CFGR &= (uint16_t)(~DMA_CFGR1_EN);
    DMA_CH_UART_TX->CNTR = g_uart_tx_transfer_length;
    DMA_CH_UART_TX->MADDR = (uint32_t)data;

    DMA_Cmd(DMA_CH_UART_TX, ENABLE); /* USART3 Tx */
}
