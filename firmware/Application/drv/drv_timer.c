#include "drv_timer.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_tim.h"
#include "ch32v30x_rcc.h"

/*
    DCDC_TRIG   --> PA1   --> TIM2 Channel2    partial mapping mode 2
    DCDC_PULSE  --> PB10  --> TIM2 Channel3
    DCDC_BLANK  --> PB8   --> TIM8 Channel3    fullly mapping

    DCDC_TRIG is enternal trigger signal, rising edge active.
    DCDC_PULSE generate a fixed time high level pulse after trigger.
    DCDC_BLANK generate a fixed time low level pulse after trigger.

    TIM2  : trigger and onepulse output
            TI2FP2 trigger --> update event TRGO output
    TIM8  : onepulse output
            TIM2 TRGO --> TRGI ITR1 trigger
    TIM10 : pulse counter
            TIM2 TRGO --> TRGI ITR1 trigger
*/

/**
 * @brief Pulse timer init.
 * 
 */
void drv_timer_pulse_init(uint16_t period)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};
    TIM_ICInitTypeDef TIM_ICInitStruct = {0};
    TIM_OCInitTypeDef TIM_OCInitStruct = {0};

    // Init clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB | 
                           RCC_APB2Periph_AFIO, ENABLE);

    // Init pulse output pin PB10 as AF PP
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Init input trigger pin PA1 as Float IN
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Apply AFIO remapping
    GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);

    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_Prescaler = 0;
    TIM_TimeBaseStruct.TIM_Period = period - 1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter = 0x1;
    TIM_ICInit(TIM2, &TIM_ICInitStruct);

    // set external trigger slave mode
    TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Trigger);

    // set trigger source
    TIM_SelectInputTrigger(TIM2, TIM_TS_TI2FP2);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 1;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM2, &TIM_OCInitStruct);


    // set TRGO to trigger other timer
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);
}

/**
 * @brief Pulse timer enable.
 * 
 */
void drv_timer_pulse_enable(void)
{
    TIM_Cmd(TIM2, ENABLE);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

/**
 * @brief Pulse timer disable.
 * 
 */
void drv_timer_pulse_disable(void)
{
    TIM_CtrlPWMOutputs(TIM2, DISABLE);
    TIM_Cmd(TIM2, DISABLE);
}

/**
 * @brief Blanking timer init.
 * 
 */
void drv_timer_blank_init(uint16_t period)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_OCInitTypeDef TIM_OCInitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};

    // Init clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | 
                           RCC_APB2Periph_AFIO, ENABLE);

    // Init blasnking output pin PB8 as AF PP
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Apply AFIO remapping
    GPIO_PinRemapConfig(GPIO_Remap_TIM8, ENABLE);

    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_Prescaler = 0;
    TIM_TimeBaseStruct.TIM_Period = period - 1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStruct);

    // set external trigger slave mode
    TIM_SelectSlaveMode(TIM8, TIM_SlaveMode_Trigger);

    // set trigger source
    TIM_SelectInputTrigger (TIM8, TIM_TS_ITR1);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 1;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM8, &TIM_OCInitStruct);

    // TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_SelectOnePulseMode (TIM8, TIM_OPMode_Single);
}

/**
 * @brief Blanking timer enable.
 * 
 */
void drv_timer_blank_enable(void)
{
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}

/**
 * @brief Blanking timmer disable.
 * 
 */
void drv_timer_blank_disable(void)
{
    TIM_Cmd(TIM8, DISABLE);
    TIM_CtrlPWMOutputs(TIM8, DISABLE);
}


/**
 * @brief Counter timer init.
 * 
 */
void drv_timer_count_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;

    // Init clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

    TIM_TimeBaseStruct.TIM_Prescaler = 0;
    TIM_TimeBaseStruct.TIM_Period = 65536 - 1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStruct);

    TIM_SelectSlaveMode(TIM10, TIM_SlaveMode_External1);
    TIM_SelectInputTrigger(TIM10, TIM_TS_ITR1);
}

/**
 * @brief Counter timer enable.
 * 
 */
void drv_timer_count_enable(void)
{
    TIM_Cmd(TIM10, ENABLE);
}

/**
 * @brief Counter timer disable.
 * 
 */
void drv_timer_count_disable(void)
{
    TIM_Cmd(TIM10, DISABLE);
    TIM_SetCounter(TIM10, 0);
}

/**
 * @brief Get fixed pulse counter.
 * 
 * @return uint16_t pulse counter.
 */
uint16_t drv_timer_count_getcnt(void)
{
    return TIM_GetCounter(TIM10);
}

/**
 * @brief Clear pulse counter.
 * 
 */
void drv_timer_count_clearcnt(void)
{
    TIM_SetCounter(TIM10, 0);
}