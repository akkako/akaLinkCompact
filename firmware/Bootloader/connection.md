# 硬件连接

网络名       GPIO      复用功能        说明

LDO_FB      --> PA0   --> ADC_IN0
DCDC_FB     --> PB0   --> ADC_IN8

DCDC_TRIG   --> PA1   --> TIM2_CH2    TIM2_RM = 10 部分映射
DCDC_PULSE  --> PB10  --> TIM2_CH3
DCDC_GATE   --> PB8   --> TIM8_CH3    TIM8_RM = 1 重映射

DCDC_VREF   --> PA4   --> DAC_OUT1

R_CALI_1    --> PA11  --> GPIO
R_CALI_2    --> PA12  --> GPIO

JTAG_TCK    --> PA5   --> SPI1_SCLK    SPI1_RM = 0 默认映射
JTAG_TCK    --> PB13  --> SPI2_SCLK    

SWDIR       --> PB2   --> GPIO
JTAG_TMSI   --> PB14  --> SPI2_MISO
JTAG_TMSO   --> PB15  --> SPI2_MOSI

JTAG_TDI    --> PA6   --> SPI1_MISO

JTAG_TDO    --> PA7   --> SPI1_MOSI
JTAG_TDO    --> PB1   --> UART4_RX     USART4_RM = x1 重映射

JTAG_SRST   --> PB9   --> GPIO
JTAG_TRST   --> PB11  --> GPIO

UART2_TX    --> PA2   --> UART2_TX    USART2_RM = 0 默认映射
UART2_RX    --> PA3   --> UART2_RX

USBHS-      --> PB6   --> USBHS_DM
USBHS+      --> PB7   --> USBHS_DP

FLASH_CS    --> PA15  --> SPI3_CS     SPI3_RM = 0 默认映射
FLASH_SCLK  --> PB3   --> SPI3_SCK
FLASH_MISO  --> PB4   --> SPI3_MISO
FLASH_MOSI  --> PB5   --> SPI3_MOSI

LOG         --> PA9   --> UART1_TX     USART1_RM = 00 默认映射

LED1        --> PC15  --> GPIO
LED2        --> PA8   --> GPIO

KEY         --> PC13  --> GPIO

5V_EN       --> PC14  --> GPIO
PWR_EN      --> PB12  --> GPIO


# Bootloader Strap 方式

1. 启动时检测外部按键按下
2. 应用程序主动跳转
3. 应用程序看门狗复位
4. 应用程序固件校验失败