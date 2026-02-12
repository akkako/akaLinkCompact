#include "drv_systick.h"
#include "drv_clock.h"
#include "ch32v30x.h"

volatile uint32_t systicks = 0;

void drv_systick_init(void)
{
    SysTick->SR &= ~(1 << 0);//clear State flag
    SysTick->CMP = SystemCoreClock/1000;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;

    NVIC_SetPriority(SysTicK_IRQn, 15);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

uint32_t drv_systick_millis(void)
{
    return systicks;
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
