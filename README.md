# akaLink Compact


* CMSIS-DAP v2.1 在线调试功能
  * SWD + UART 接口
  * JTAG 接口
* HScope 功能
  * SWD Host
  * RTT 终端模式
  * 乒乓模式
  * 链表模式
  * 数组模式
  * 支持vofa/serial plot格式
* 5V 输出功能


## 上位机软件功能需求

1. 配置 CMSIS-DAP 支持状态（SWD+UART/SWD+JTAG）
2. 配置 HScope 功能
   1. 调试接口速率
   2. 功能开关
   3. MEM-AP 架构配置
   4. HScope 工作模式（数组，链表，乒乓）
   5. RTT 块地址
3. 跳转 Bootloader 进行升级操作
4.
