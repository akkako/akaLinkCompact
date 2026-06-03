#include "drv_flash.h"
#include "ch32v30x_flash.h"
#include "ch32v30x_rcc.h"

/**
 * @brief 从 Flash 内读取数据
 * @param buff 读取缓冲区
 * @param addr 读取开始地址
 * @param length 读取长度
 */
extern void drv_flash_read(uint8_t *buff, uint32_t addr, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        buff[i] = ((uint8_t *)addr)[i];
    }
}

/**
 * @brief 向 Flash 内写入数据
 * @param buff 写入缓冲区
 * @param addr 写入地址（需要为页起始地址，256字节对齐）
 * @param length 写入长度（需要 2 字节对齐、最大为页大小 2048）
 * @return 写入状态（0 为成功，-1 为失败）
 */
extern int drv_flash_write(uint8_t *buff, uint32_t addr, uint32_t length)
{
    FLASH_Status ret;

    FLASH_Unlock();
    ret = FLASH_ErasePage(addr);
    if (ret != FLASH_COMPLETE)
    {
        goto error;
    }

    for (uint32_t i = 0; i < length / 2; i++)
    {
        ret = FLASH_ProgramHalfWord(addr, ((uint16_t *)buff)[i]);
        addr += 2;
        if (ret != FLASH_COMPLETE)
        {
            goto error;
        }
    }

error:
    FLASH_Lock();
    if (ret != FLASH_COMPLETE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}