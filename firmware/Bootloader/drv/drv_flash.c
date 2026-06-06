#include "drv_flash.h"
#include "ch32v30x_flash.h"
#include "ch32v30x_rcc.h"

/**
 * @brief 从 Flash 内读取数据
 * @param buff 读取缓冲区
 * @param addr 读取开始地址
 * @param length 读取长度
 */
extern void drv_flash_read (uint8_t *buff, uint32_t addr, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        buff[i] = ((uint8_t *)addr)[i];
    }
}

/**
 * @brief 向 Flash 内写入数据
 * @param buff 写入缓冲区
 * @param addr 写入地址（需要为页起始地址，256字节对齐）
 * @param length 写入长度（需要 4 字节对齐）
 * @return 写入状态（0 为成功，-1 为失败）
 */
extern int drv_flash_write (uint8_t *buff, uint32_t addr, uint32_t length) {
    FLASH_Unlock_Fast();
    FLASH_Access_Clock_Cfg (FLASH_Access_SYSTEM_HALF);
    for (uint32_t pos = addr; pos < addr + length; pos += 256) {
        FLASH_ErasePage_Fast (pos);
        FLASH_ProgramPage_Fast (pos, (uint32_t *)(&buff[pos - addr]));
    }
    FLASH_Lock_Fast();

    return 0;
}