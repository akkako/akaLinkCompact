# 硬件连接


| 引脚名称 | 引脚定义 | 功能1      | 功能2      | 引脚复用  |
| -------- | -------- | ---------- | ---------- | --------- |
| ADCV     | PA1      |            |            | ADC_CH1   |
| TRST     | PC6      | JTAG_TRST  | N/A(INPUT) | GPIO      |
| LED      | PC7      |            |            | GPIO      |
| SRST     | PC8      |            |            | GPIO      |
| 5VEN     | PC9      |            |            | GPIO      |
| TDI      | PB10     | JTAG_TDI   | UART_TXD   | UART3_TXD |
| TDO      | PB11     | JTAG_TDO   | UART_RXD   | UART3_RXD |
| SWDIR    | PB12     | N/A(HIGH)  | SWDIR      | GPIO      |
| TCK      | PB13     | JTAG_TCK   | SWCLK      | SPI2_SCLK |
| TMSI     | PB14     | N/A(INPUT) | SWDI       | SPI2_MISO |
| TMSO     | PB15     | JTAG_TMS   | SWDO       | SPI2_MOSI |
|          |          |            |            |           |
