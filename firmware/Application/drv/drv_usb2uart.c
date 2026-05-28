#include "drv_usb2uart.h"

#include "ch32v30x_dma.h"
#include "ch32v30x_rcc.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_usart.h"
#include "ch32v30x_misc.h"

#include "usb_main.h"
#include "drv_print.h"

#define USART_RX_BUFFER_SIZE (512)

// PB10  UART3_TX DMA_CH2
// PB11  UART3_RX DMA_CH3

#define DMA_CH_UART_TX DMA1_Channel2
#define DMA_CH_UART_RX DMA1_Channel3

// 串口 DMA 接收双缓冲
uint32_t usart_rx_buffer_index = 0;
uint8_t usart_rx_buffer[2][USART_RX_BUFFER_SIZE];

// 串口 DMA 发送计数
uint32_t usart_tx_length = 0;

void DMA1_Channel2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void DMA1_Channel3_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void USART3_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

/**
 * @brief UART3 DMA 发送完成中断处理函数
 * @param None
 */
void DMA1_Channel2_IRQHandler (void) {
    if (DMA1->INTFR & DMA_TCIF2) {
        // 关闭 DMA
        drv_usb2uart_stop_tx_dma();
        // 通知上层发送完成
        chry_dap_usb2uart_uart_send_complete (usart_tx_length);
    }
}

/**
 * @brief UART3 DMA 接收完成中断处理函数
 * @param None
 */
void DMA1_Channel3_IRQHandler (void) {
    if (DMA1->INTFR & DMA_TCIF3) {
        // 关闭 DMA
        drv_usb2uart_stop_rx_dma();
        // 上一个接收缓冲区存满了，重新启动接收 DMA 切下一个缓冲区
        usart_rx_buffer_index = !usart_rx_buffer_index;
        drv_usb2uart_start_rx_dma (&usart_rx_buffer[usart_rx_buffer_index][0], USART_RX_BUFFER_SIZE);
        // 将已经接收的数据拷贝到接收 FIFO 中，长度固定为 512
        chry_ringbuffer_overwrite (&g_uartrx, usart_rx_buffer[!usart_rx_buffer_index], USART_RX_BUFFER_SIZE);

        // 检查是否有 IDLE 中断，如果有则清除
        if (USART_GetITStatus (USART3, USART_IT_IDLE)) {
            (void)USART3->STATR;
            (void)USART3->DATAR;
            NVIC_ClearPendingIRQ(USART3_IRQn);
        }

        if (USART_GetITStatus (USART3, USART_IT_ORE))
        {
            printf("ORE\r\n");
        }
    }
}

/**
 * @brief UART3 中断处理函数
 * @param None
 */
void USART3_IRQHandler (void) {

    if (USART_GetITStatus (USART3, USART_IT_IDLE)) {
        drv_usb2uart_stop_rx_dma();
        (void)USART3->STATR;
        (void)USART3->DATAR;
        // 获取 DMA 传输的数据长度
        uint32_t rx_len = USART_RX_BUFFER_SIZE - DMA_CH_UART_RX->CNTR;
        // 空闲中断，说明接收断帧，重新启动接收 DMA 切到下一个缓冲区
        usart_rx_buffer_index = !usart_rx_buffer_index;
        drv_usb2uart_start_rx_dma (&usart_rx_buffer[usart_rx_buffer_index][0], USART_RX_BUFFER_SIZE);
        // 将已接收的数据拷贝到接收 FIFO 中
        chry_ringbuffer_overwrite (&g_uartrx, usart_rx_buffer[!usart_rx_buffer_index], rx_len);
    }
}

void drv_usb2uart_gpio_af_uart (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    /* USART3 TX--> PB10   RX -->PB11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init (GPIOB, &GPIO_InitStructure);
}

void drv_usb2uart_stop_rx_dma (void) {
    // 关闭 DMA
    DMA_CH_UART_RX->CFGR = 0;
    // 清除接收通道 DMA 标志
    DMA1->INTFCR = DMA_CGIF3 | DMA_CTCIF3 | DMA_CHTIF3 | DMA_CTEIF3;
}

void drv_usb2uart_start_rx_dma (uint8_t *data, uint16_t len) {
    drv_usb2uart_stop_rx_dma();

    // 启动 DMA 传输
    DMA_CH_UART_RX->CNTR = len;
    DMA_CH_UART_RX->MADDR = (uint32_t)data;
    DMA_CH_UART_RX->PADDR = (uint32_t)(&USART3->DATAR);
    DMA_CH_UART_RX->CFGR =
        DMA_CFGR1_EN |    // 通道使能
        DMA_CFGR1_TCIE |  // 传输完成触发中断
        DMA_CFGR1_MINC |  // 内存地址自增
        DMA_CFGR1_PL;     // 优先级最高
}

void drv_usb2uart_stop_tx_dma (void) {
    // 关闭 DMA
    DMA_CH_UART_TX->CFGR = 0;
    // 清除发送通道 DMA 标志
    DMA1->INTFCR = DMA_CGIF2 | DMA_CTCIF2 | DMA_CHTIF2 | DMA_CTEIF2;
}

void drv_usb2uart_start_tx_dma (uint8_t *data, uint16_t len) {
    usart_tx_length = len;
    drv_usb2uart_stop_tx_dma();
    // 启动 DMA 传输
    DMA_CH_UART_TX->CNTR = usart_tx_length;
    DMA_CH_UART_TX->MADDR = (uint32_t)data;
    DMA_CH_UART_TX->PADDR = (uint32_t)(&USART3->DATAR);
    DMA_CH_UART_TX->CFGR =
        DMA_CFGR1_EN |    // 通道使能
        DMA_CFGR1_TCIE |  // 传输完成触发中断
        DMA_CFGR1_MINC |  // 内存地址自增
        DMA_CFGR1_DIR |   // 内存到外设
        DMA_CFGR1_PL;     // 优先级最高
}

void drv_usb2uart_preinit (void) {
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB1PeriphClockCmd (RCC_APB1Periph_USART3, ENABLE);
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    // drv_usb2uart_gpio_af_uart();

    // 初始化串口设置
    drv_usb2uart_set_linecoding (115200, 0, 0, 8);

    // 开启 DMA 通道 2 全局中断使能（DMA 发送完成中断）
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    // 开启 DMA 通道 3 全局中断使能（DMA 接收完成中断）
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    // 开启 USART3 全局中断使能（UART 接收空闲中断）
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    drv_usb2uart_enable (1);
    usart_rx_buffer_index = 0;
    drv_usb2uart_start_rx_dma (&usart_rx_buffer[usart_rx_buffer_index][0], USART_RX_BUFFER_SIZE);
}

void drv_usb2uart_enable (uint8_t enable) {
    USART_Cmd (USART3, enable ? ENABLE : DISABLE);
}

void drv_usb2uart_set_linecoding (uint32_t dwDTERate, uint8_t bCharFormat, uint8_t bParityType, uint8_t bDataBits) {
    USART_InitTypeDef USART_InitStructure = {0};

    // drv_usb2uart_stop_rx_dma();
    // drv_usb2uart_stop_tx_dma();

    USART_DeInit (USART3);
    USART_InitStructure.USART_BaudRate = dwDTERate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    /* Number of stop bits (0: 1 stop bit; 1: 1.5 stop bits; 2: 2 stop bits). */
    if (bCharFormat == 1) {
        USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
    } else if (bCharFormat == 2) {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    } else {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    /* Check digit (0: None; 1: Odd; 2: Even; 3: Mark; 4: Space); */
    if (bParityType == 1) {
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    } else if (bParityType == 2) {
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    } else {
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init (USART3, &USART_InitStructure);
    USART_Cmd (USART3, ENABLE);
    USART_ITConfig (USART3, USART_IT_IDLE, ENABLE);
    USART_DMACmd (USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
}
