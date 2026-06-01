# Custom HID 通信协议

## HID 数据包

固定长度64字节

Byte[0x01] = Report ID(0x01 Request, 0x02 Response)
Byte[0x02] = Data Length
Byte[0x03] = Command type
Byte[0x04] = Command data1
Byte[0x04] = Command data2

## 指令类型

1. 输出模式设置指令/获取指令
   0x01/0x81
   SWD + JTAG
   SWD + VCOM
2. SWD 模拟方式设置指令/获取指令
   0x02/0x82
   GPIO 模拟
   SPI 模拟
3. 5V输出使能设置指令/获取指令
   0x03/0x83
   开启5V输出使能
   关闭5V输出使能
4. 获取 Target 电压获取指令
   0x84
5. 保存当前配置设置指令
   0x05
6. 进入DFU模式设置指令
   0x06

## 存储数据

```c
typedef struct _app_param_t
{
    uint8_t output_mode;
    uint8_t swd_sim_mode;
    uint8_t usb5v_out_mode;
    uint8_t padding;
} app_param_t
```
