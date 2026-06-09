from datetime import datetime
import binascii
import os
import struct
import sys

args = sys.argv[1:]

if args[0] == "mrs":
    app_filename = "akaLink_Compact_Dfu.bin"
    output_filename = "akaLink_Compact_Dfu_pack.bin"
elif args[0] == "make":
    app_filename = "build_exec/akaLink_Compact_Dfu.bin"
    output_filename = "build_exec/akaLink_Compact_Dfu_pack.bin"
else :
    exit()

bl_ver_str = '1.02'
hw_ver_str = 'A.01'

# 地址与长度常量
FIRMWARE_MAX_LEN = 0x3EE4      # 固件最大长度：16100 bytes
BL_VER_OFFSET    = 0x3EE4      # 版本字符串偏移
BL_VER_LEN       = 8           # 版本字符串长度
TIME_OFFSET      = 0x3EEC      # 编译时间偏移
TIME_LEN         = 20          # 编译时间总长度（含末尾 0x00）
TOTAL_SIZE       = 0x4000

HW_VER_OFFSET    = 0x3F00      # 硬件版本字符串偏移
HW_VER_LEN       = 8           # 硬件版本字符串长度
PROD_TIME_OFFSET = 0x3F08      # 硬件生产时间
PROD_TIME_LEN    = 20          # 硬件生产时间总长度（含末尾 0x00）

# 获取当前时间
current_time = datetime.now()
formatted_time = current_time.strftime("%Y/%m/%d %H:%M:%S")

# 读取固件并截断
with open(app_filename, "rb") as f:
    app_content = f.read()
app_content = app_content[:FIRMWARE_MAX_LEN]  # 多余截断

# 处理版本字符串：固定 8 字节，不足补0，超出截断
bl_ver_bytes = bl_ver_str.encode("ASCII")
if len(bl_ver_bytes) > BL_VER_LEN:
    bl_ver_bytes = bl_ver_bytes[:BL_VER_LEN]
else:
    bl_ver_bytes = bl_ver_bytes.ljust(BL_VER_LEN, b'\x00')
    
# 处理硬件版本字符串：固定 8 字节，不足补0，超出截断
hw_ver_bytes = hw_ver_str.encode("ASCII")
if len(hw_ver_bytes) > HW_VER_LEN:
    hw_ver_bytes = hw_ver_bytes[:HW_VER_LEN]
else:
    hw_ver_bytes = hw_ver_bytes.ljust(HW_VER_LEN, b'\x00')

# 处理编译时间：固定 20 字节，最后一位必须为 0x00
# 时间字符串最多占前 19 字节，剩余字节补 0x00
time_str_bytes = formatted_time.encode("ASCII")
if len(time_str_bytes) > TIME_LEN - 1:
    time_str_bytes = time_str_bytes[:TIME_LEN - 1]
# 组装 20 字节：时间字符串 + 0x00 填充
time_bytes = time_str_bytes + b'\x00' * (TIME_LEN - len(time_str_bytes))

# 构建输出缓冲区，初始全部填充 0xFF
output = bytearray([0xFF] * TOTAL_SIZE)

# 在指定偏移写入数据
output[0:len(app_content)] = app_content
output[BL_VER_OFFSET:BL_VER_OFFSET + BL_VER_LEN] = bl_ver_bytes
output[TIME_OFFSET:TIME_OFFSET + TIME_LEN] = time_bytes
output[HW_VER_OFFSET:HW_VER_OFFSET + HW_VER_LEN] = hw_ver_bytes
output[PROD_TIME_OFFSET:PROD_TIME_OFFSET + PROD_TIME_LEN] = time_bytes

# 保存文件
with open(output_filename, "wb") as f:
    f.write(output)

print(f"boot ver str: {bl_ver_str}")
print(f"compile time: {formatted_time}")
print(f"output size : {len(output)} bytes (0x{len(output):04X})")