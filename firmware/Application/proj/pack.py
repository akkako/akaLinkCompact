from datetime import datetime
import binascii
import os
import struct

app_filename = "akaLink.bin"
output_filename = "akaLink_pack.bin"

desc_str = 'CMSIS-DAP'
fw_ver_str = '0.02'

# 地址与长度常量
FIRMWARE_MAX_LEN = 0x17F00     # 固件最大长度：98048 bytes

APP_LENGTH_OFFSET   = 0x0000      # 固件长度偏移
APP_LENGTH_LEN   = 4      # 固件长度偏移
APP_CRC32_OFFSET   = 0x0004      # 固件CRC32偏移
APP_CRC32_LEN   = 4      # 固件CRC32偏移

FW_VER_OFFSET   = 0x0008      # 版本字符串偏移
FW_VER_LEN      = 8           # 版本字符串长度
TIME_OFFSET      = 0x0010      # 编译时间偏移
TIME_LEN         = 20          # 编译时间总长度（含末尾 0x00）
DESC_STR_OFFSET = 0x0024
DESC_STR_LEN    = 24

APP_CODE_OFFSET = 0x0100


# 获取当前时间
current_time = datetime.now()
formatted_time = current_time.strftime("%Y/%m/%d %H:%M:%S")

app_len = os.path.getsize(app_filename)

# 读取固件并截断
with open(app_filename, "rb") as f:
    app_content = f.read()
app_content = app_content[:FIRMWARE_MAX_LEN]  # 多余截断

# 计算 CRC32
crc32_value = 0x0D000721
crc32_value = binascii.crc32(app_content, crc32_value)

# 处理软件版本字符串：固定 8 字节，不足补0，超出截断
fw_ver_bytes = fw_ver_str.encode("ASCII")
if len(fw_ver_bytes) > FW_VER_LEN:
    fw_ver_bytes = fw_ver_bytes[:FW_VER_LEN]
else:
    fw_ver_bytes = fw_ver_bytes.ljust(FW_VER_LEN, b'\x00')

# 处理软件描述字符串：固定 24 字节，不足补0，超出截断
desc_bytes = desc_str.encode("ASCII")
if len(desc_bytes) > DESC_STR_LEN:
    desc_bytes = desc_bytes[:DESC_STR_LEN]
else:
    desc_bytes = desc_bytes.ljust(DESC_STR_LEN, b'\x00')


# 处理编译时间：固定 20 字节，最后一位必须为 0x00
# 时间字符串最多占前 19 字节，剩余字节补 0x00
time_str_bytes = formatted_time.encode("ASCII")
if len(time_str_bytes) > TIME_LEN - 1:
    time_str_bytes = time_str_bytes[:TIME_LEN - 1]
# 组装 20 字节：时间字符串 + 0x00 填充
time_bytes = time_str_bytes + b'\x00' * (TIME_LEN - len(time_str_bytes))

# 构建输出缓冲区，初始全部填充 0xFF
output = bytearray([0xFF] * (app_len+256))

# 在指定偏移写入数据

output[APP_LENGTH_OFFSET:APP_LENGTH_OFFSET + APP_LENGTH_LEN] = struct.pack('<I', app_len)
output[APP_CRC32_OFFSET:APP_CRC32_OFFSET + APP_CRC32_LEN] = struct.pack('<I', crc32_value)
output[FW_VER_OFFSET:FW_VER_OFFSET + FW_VER_LEN] = fw_ver_bytes
output[TIME_OFFSET:TIME_OFFSET + TIME_LEN] = time_bytes
output[DESC_STR_OFFSET:DESC_STR_OFFSET + DESC_STR_LEN] = desc_bytes
output[APP_CODE_OFFSET:APP_CODE_OFFSET + len(app_content)] = app_content

# 保存文件
with open(output_filename, "wb") as f:
    f.write(output)

print(f"version str : {fw_ver_str}")
print(f"compile time: {formatted_time}")
print(f"output size : {len(output)} bytes (0x{len(output):04X})")