#include "drv_systick.h"
#include "drv_clock.h"
#include "ch32v30x.h"

volatile uint32_t systicks = 0;

/**
 * @brief 初始化SysTick定时器
 * 
 */
void drv_systick_init(void)
{
    SysTick->SR &= ~(1 << 0);//clear State flag
    SysTick->CMP = SystemCoreClock/1000;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;

    NVIC_SetPriority(SysTicK_IRQn, 15);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

/**
 * @brief 获取当前系统时间（单位：毫秒）
 * 
 * @return uint32_t 当前系统时间（毫秒）
 */
uint32_t drv_systick_millis(void)
{
    return systicks;
}

/**
 * @brief 延时指定毫秒数
 * 
 * @param ms 延时时间（毫秒）
 */
void drv_systick_delay_ms(uint32_t ms)
{
    uint32_t end = drv_systick_millis() + ms;
    while(drv_systick_millis() < end);
}


void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    if(SysTick->SR == 1)
    {
        SysTick->SR = 0;
        systicks++;
    }
}
