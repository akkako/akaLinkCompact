from datetime import datetime
import binascii
import os
import struct

app_filename = "akaLink.bin"
output_filename = "akaLink_m.bin"

desc_str = 'CMSIS-DAP v2.1'

# get current time and format
current_time = datetime.now()
formatted_time = current_time.strftime("%Y/%m/%d %H:%M:%S")

# get firmware file size
app_len = os.path.getsize(app_filename)

# read firmware file content
with open(app_filename, "rb") as f:
    app_content = f.read()

# calculate crc32
crc32_value = 0x0D000721
crc32_value = binascii.crc32(app_content, crc32_value)

# save to new file
with open(output_filename, "wb") as f:
    # add file size
    f.write(struct.pack('<I', app_len))
    # add file crc32
    f.write(struct.pack('<I', crc32_value))
    # add compile time
    f.write(formatted_time.encode("ASCII"))
    f.write(struct.pack('<B', 0x00))
    # add version string
    f.write(version_str.encode("ASCII"))


    # add firmware content
    f.write(app_content)
    


print(f"version str : {version_str}")
print(f"compile time: {formatted_time}")
print(f"file crc32  : {hex(crc32_value)}")
print(f"file size   : {app_len}")
