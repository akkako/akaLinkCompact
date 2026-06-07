#include "flash_fat16.h"
#include "usbd_core.h"
#include <strings.h>
#include "drv_flash.h"
#include "sys_define.h"

#define FAT16_BOOT_SECTOR_SIZE (512)
#define FAT16_TABLE_SIZE (5)
#define FAT16_FILENAME_SIZE (11)
#define FAT16_DIR_SIZE (64)

#define FLASH_FAT16_BOOT_SECTOR_ADDR (0x00000000)
#define FLASH_FAT16_1_ADDR (0x1000)
#define FLASH_FAT16_2_ADDR (0x1800)
#define FLASH_FAT16_ROOT_ADDR (0x2000)
#define FLASH_FAT16_FILE_START_ADDR (0x6000)

#define FILE_SUFFIX1_LEN (3)
#define FILE_SUFFIX2_LEN (3)
#define FILE_SUFFIX1_NAME "BIN"
#define FILE_SUFFIX2_NAME "bin"

extern uint8_t g_app_download_finished;
extern uint8_t g_app_verify_state;
extern uint8_t *g_app_compile_time;
extern uint8_t *g_app_version_str;

typedef struct
{
    char file_name[11];
    uint8_t attr;
    uint8_t res;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_data;
    uint16_t last_access_data;
    uint16_t clus_high;
    uint16_t write_time;
    uint16_t write_data;
    uint16_t clus_low;
    uint32_t file_size;
} fat_dir_type;

typedef struct
{
    uint8_t attr;
    char unicode_1[10];
    uint8_t long_name_mark;
    uint8_t reserved;
    uint8_t check;
    char unicode_2[12];
    uint16_t start_clus;
    char unicode_3[4];
} fat_long_name_type;

// DBR
const uint8_t fat16_boot_sector[FAT16_BOOT_SECTOR_SIZE] =
    {
        0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x08, 0x01, 0x02, 0x00,
        0x02, 0x00, 0x02, 0xC8, 0x00, 0xF8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0x8B, 0xEE, 0xEB, 0x78, 0x4E, 0x4F, 0x20, 0x4E, 0x41,
        0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x33, 0xC9,
        0x8E, 0xD1, 0xBC, 0xF0, 0x7B, 0x8E, 0xD9, 0xB8, 0x00, 0x20, 0x8E, 0xC0, 0xFC, 0xBD, 0x00, 0x7C,
        0x38, 0x4E, 0x24, 0x7D, 0x24, 0x8B, 0xC1, 0x99, 0xE8, 0x3C, 0x01, 0x72, 0x1C, 0x83, 0xEB, 0x3A,
        0x66, 0xA1, 0x1C, 0x7C, 0x26, 0x66, 0x3B, 0x07, 0x26, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA,
        0x02, 0x88, 0x56, 0x02, 0x80, 0xC3, 0x10, 0x73, 0xEB, 0x33, 0xC9, 0x8A, 0x46, 0x10, 0x98, 0xF7,
        0x66, 0x16, 0x03, 0x46, 0x1C, 0x13, 0x56, 0x1E, 0x03, 0x46, 0x0E, 0x13, 0xD1, 0x8B, 0x76, 0x11,
        0x60, 0x89, 0x46, 0xFC, 0x89, 0x56, 0xFE, 0xB8, 0x20, 0x00, 0xF7, 0xE6, 0x8B, 0x5E, 0x0B, 0x03,
        0xC3, 0x48, 0xF7, 0xF3, 0x01, 0x46, 0xFC, 0x11, 0x4E, 0xFE, 0x61, 0xBF, 0x00, 0x00, 0xE8, 0xE6,
        0x00, 0x72, 0x39, 0x26, 0x38, 0x2D, 0x74, 0x17, 0x60, 0xB1, 0x0B, 0xBE, 0xA1, 0x7D, 0xF3, 0xA6,
        0x61, 0x74, 0x32, 0x4E, 0x74, 0x09, 0x83, 0xC7, 0x20, 0x3B, 0xFB, 0x72, 0xE6, 0xEB, 0xDC, 0xA0,
        0xFB, 0x7D, 0xB4, 0x7D, 0x8B, 0xF0, 0xAC, 0x98, 0x40, 0x74, 0x0C, 0x48, 0x74, 0x13, 0xB4, 0x0E,
        0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEF, 0xA0, 0xFD, 0x7D, 0xEB, 0xE6, 0xA0, 0xFC, 0x7D, 0xEB,
        0xE1, 0xCD, 0x16, 0xCD, 0x19, 0x26, 0x8B, 0x55, 0x1A, 0x52, 0xB0, 0x01, 0xBB, 0x00, 0x00, 0xE8,
        0x3B, 0x00, 0x72, 0xE8, 0x5B, 0x8A, 0x56, 0x24, 0xBE, 0x0B, 0x7C, 0x8B, 0xFC, 0xC7, 0x46, 0xF0,
        0x3D, 0x7D, 0xC7, 0x46, 0xF4, 0x29, 0x7D, 0x8C, 0xD9, 0x89, 0x4E, 0xF2, 0x89, 0x4E, 0xF6, 0xC6,
        0x06, 0x96, 0x7D, 0xCB, 0xEA, 0x03, 0x00, 0x00, 0x20, 0x0F, 0xB6, 0xC8, 0x66, 0x8B, 0x46, 0xF8,
        0x66, 0x03, 0x46, 0x1C, 0x66, 0x8B, 0xD0, 0x66, 0xC1, 0xEA, 0x10, 0xEB, 0x5E, 0x0F, 0xB6, 0xC8,
        0x4A, 0x4A, 0x8A, 0x46, 0x0D, 0x32, 0xE4, 0xF7, 0xE2, 0x03, 0x46, 0xFC, 0x13, 0x56, 0xFE, 0xEB,
        0x4A, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A, 0x10, 0x91, 0x8B, 0x46, 0x18, 0x96, 0x92, 0x33,
        0xD2, 0xF7, 0xF6, 0x91, 0xF7, 0xF6, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8,
        0xC0, 0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42,
        0x8B, 0xF4, 0x8A, 0x56, 0x24, 0xCD, 0x13, 0x61, 0x61, 0x72, 0x0B, 0x40, 0x75, 0x01, 0x42, 0x03,
        0x5E, 0x0B, 0x49, 0x75, 0x06, 0xF8, 0xC3, 0x41, 0xBB, 0x00, 0x00, 0x60, 0x66, 0x6A, 0x00, 0xEB,
        0xB0, 0x42, 0x4F, 0x4F, 0x54, 0x4D, 0x47, 0x52, 0x20, 0x20, 0x20, 0x20, 0x0D, 0x0A, 0x52, 0x65,
        0x6D, 0x6F, 0x76, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x73, 0x20, 0x6F, 0x72, 0x20, 0x6F, 0x74,
        0x68, 0x65, 0x72, 0x20, 0x6D, 0x65, 0x64, 0x69, 0x61, 0x2E, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73,
        0x6B, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x50, 0x72, 0x65, 0x73, 0x73, 0x20,
        0x61, 0x6E, 0x79, 0x20, 0x6B, 0x65, 0x79, 0x20, 0x74, 0x6F, 0x20, 0x72, 0x65, 0x73, 0x74, 0x61,
        0x72, 0x74, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xCB, 0xD8, 0x55, 0xAA};

// root dir
const uint8_t fat16_root_dir_sector[FAT16_DIR_SIZE] = {
    'a', 'k', 'a', 'L', 'i', 'n', 'k', 'D', 'F', 'U', 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBC, 0x41, 0x23, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    'I', 'N', 'F', 'O', 0x20, 0x20, 0x20, 0x20, 0x54, 0x58, 0x54, 0x20, 0x00, 0x18, 0xBD, 0x41,
    0x23, 0x59, 0x23, 0x59, 0x00, 0x00, 0x7B, 0x3D, 0x23, 0x59, 0x02, 0x00, 172, 0x00, 0x00, 0x00};

// FAT table
const uint8_t fat16_table_sector0[FAT16_TABLE_SIZE] =
    {
        0xF8,
        0xFF,
        0xFF,
        0xFF,
        0x0F,
};

uint8_t vfat16_file_data[] =
    "SN:000000000000000000000000\r\n"
    "Desc:Unknown Firmware        \r\n"
    "HWVER:Unknown \r\n"
    "BLVER:Unknown \r\n"
    "FWVER:Unknown \r\n"
    "CRC32 Check:fail\r\n"
    "Please drag new firmware into here to upgrade.";

static fat_dir_type g_file_attr;

static uint32_t file_write_nr = 0;

static uint32_t fat16_memory_cmp (uint8_t *dst, uint8_t *src, uint32_t len);
static uint32_t fat16_memory_copy (uint8_t *dst, const uint8_t *src, uint32_t len);
static uint32_t fat16_memory_memset (uint8_t *dst, uint8_t set, uint32_t len);

uint32_t flash_fat16_root_dir_write (uint32_t fat_lbk, uint8_t *data, uint32_t len);
uint32_t flash_fat16_sector_write (uint32_t fat_lbk, uint8_t *data, uint32_t len);

uint32_t flash_write_data (uint32_t address, uint8_t *data, uint32_t len);
uint32_t flash_read_data (uint32_t address, uint8_t *data, uint32_t len);

static uint32_t fat16_memory_copy (uint8_t *dst, const uint8_t *src, uint32_t len) {
    uint32_t i_index;
    for (i_index = 0; i_index < len; i_index++) {
        dst[i_index] = src[i_index];
    }
    return i_index;
}

static uint32_t fat16_memory_memset (uint8_t *dst, uint8_t set, uint32_t len) {
    for (uint32_t i_index = 0; i_index < len; i_index++) {
        dst[i_index] = (uint8_t)set;
    }
    return len;
}

static uint32_t fat16_memory_cmp (uint8_t *dst, uint8_t *src, uint32_t len) {
    uint32_t i_index;
    for (i_index = 0; i_index < len; i_index++) {
        if (dst[i_index] != src[i_index])
            return 1;
    }
    return 0;
}

uint32_t flash_fat16_read (uint32_t fat_lbk, uint8_t *data, uint32_t len) {
    fat16_memory_memset (data, 0, VFAT16_CONF_SECTOR_SIZE);
    switch (fat_lbk) {
    case FLASH_FAT16_BOOT_SECTOR_ADDR:
        fat16_memory_copy (data, fat16_boot_sector, FAT16_BOOT_SECTOR_SIZE);
        break;
    case FLASH_FAT16_1_ADDR:
    case FLASH_FAT16_2_ADDR:
        fat16_memory_copy (data, fat16_table_sector0, FAT16_TABLE_SIZE);
        break;
    case FLASH_FAT16_ROOT_ADDR:
        fat16_memory_copy (data, fat16_root_dir_sector, FAT16_DIR_SIZE);
        break;
    case FLASH_FAT16_FILE_START_ADDR:
        fat16_memory_copy (data, vfat16_file_data, sizeof (vfat16_file_data));
        break;
    default:
        break;
    }
    return VFAT16_CONF_SECTOR_SIZE;
}

#if 1
static const char hex_format[] = "0123456789ABCDEF";

#define INFO_FILE_SN_OFFSET (3)
#define INFO_FILE_DESC_OFFSET (34)
#define INFO_FILE_HWVER_OFFSET (66)
#define INFO_FILE_BLVER_OFFSET (82)
#define INFO_FILE_FWVER_OFFSET (98)
#define INFO_FILE_CRC32_OFFSET (120)

size_t str_copy_limit (char *dst, const char *src, size_t max_len) {
    size_t i = 0;
    while (i < max_len && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    return i;
}

void fat16_file_init (void) {
    uint8_t *pdest;

    // Fix UID
    pdest = &vfat16_file_data[INFO_FILE_SN_OFFSET];
    for (int j = 0; j < 3; j++) {
        uint32_t IC_UID = *(uint32_t *)(0x1FFFF7E8 + j * 4);
        for (int i = 0; i < 8; i++) {
            pdest[i + j * 8] = hex_format[(IC_UID >> i * 4) & 0xF];
        }
    }

    // HWVER
    pdest = &vfat16_file_data[INFO_FILE_HWVER_OFFSET];
    memset (pdest, ' ', 8);
    str_copy_limit ((char *)pdest, (const char *)HARDWARE_VER_STR_ADDR, 8);

    // BLVER
    pdest = &vfat16_file_data[INFO_FILE_BLVER_OFFSET];
    memset (pdest, ' ', 8);
    str_copy_limit ((char *)pdest, (const char *)BOOTLOADER_VER_STR_ADDR, 8);

    // CRC32 Check State
    if (g_app_verify_state) {
        // Desc string
        pdest = &vfat16_file_data[INFO_FILE_DESC_OFFSET];
        memset (pdest, ' ', 24);
        str_copy_limit ((char *)pdest, (const char *)APPLICATION_DESC_STR_ADDR, 24);

        // Check State
        memcpy (&vfat16_file_data[INFO_FILE_CRC32_OFFSET], "pass", 4);

        // FWVER
        pdest = &vfat16_file_data[INFO_FILE_FWVER_OFFSET];
        memset (pdest, ' ', 8);
        str_copy_limit ((char *)pdest, (const char *)APPLICATION_VER_STR_ADDR, 8);
    }
}
#endif

uint32_t flash_fat16_write (uint32_t fat_lbk, uint8_t *data, uint32_t len) {
    switch (fat_lbk) {
    case FLASH_FAT16_BOOT_SECTOR_ADDR:
        break;
    case FLASH_FAT16_1_ADDR:
    case FLASH_FAT16_2_ADDR:
        break;
    case FLASH_FAT16_ROOT_ADDR:
        flash_fat16_root_dir_write (fat_lbk, data, len);
        break;
    default:
        if (fat_lbk >= FLASH_FAT16_FILE_START_ADDR) {
            flash_fat16_sector_write (fat_lbk, data, len);
        }
        break;
    }
    return len;
}

uint32_t flash_fat16_root_dir_write (uint32_t fat_lbk, uint8_t *data, uint32_t len) {
    fat_dir_type *pdir = (fat_dir_type *)data;
    fat_long_name_type *plong_name = (fat_long_name_type *)data;

    uint32_t i_index = 2, loop_len = 0;

    pdir += 2;
    plong_name += 2;
    loop_len += 64;

    while (i_index++ < 2048 / 32 && loop_len < len) {
        if (((pdir->attr & 0x20) && ((pdir + 1)->attr == 0x00)) && plong_name->long_name_mark != 0x0F && plong_name->attr != 0xe5 && pdir->file_size != 0) {
            // continue find
            break;
        } else {
            pdir++;
            plong_name++;
        }
        loop_len += 32;
    }

    if (i_index <= 2048 / 32 && loop_len < len && plong_name->long_name_mark != 0x0F && plong_name->attr != 0xe5 && pdir->file_size != 0) {
        fat16_memory_copy ((uint8_t *)&g_file_attr, (const uint8_t *)pdir, 32);
        file_write_nr = 0;
    } else {
        fat16_memory_memset ((uint8_t *)&g_file_attr, 0, 32);
    }
    return len;
}

uint32_t flash_fat16_sector_write (uint32_t fat_lbk, uint8_t *data, uint32_t len) {
    uint32_t file_size = g_file_attr.file_size;
    uint32_t status;

    if (file_size > VFAT16_FLASH_SIZE) {
        return 0;
    }

    if ((fat16_memory_cmp ((uint8_t *)&g_file_attr.file_name[8],
                           (uint8_t *)FILE_SUFFIX1_NAME, FILE_SUFFIX1_LEN) == 0) ||
        (fat16_memory_cmp ((uint8_t *)&g_file_attr.file_name[8],
                           (uint8_t *)FILE_SUFFIX2_NAME, FILE_SUFFIX2_LEN) == 0)) {

        file_size = g_file_attr.file_size;

        if ((file_write_nr + len) >= file_size) {
            len = file_size - file_write_nr;
        }

        /* write data to flash and check crc */
        status = flash_write_data (file_write_nr, data, len);

        file_write_nr += len;

        if (status == 0) {
            if (file_write_nr >= file_size) {
                /* download finish */
                file_write_nr = 0;
                g_app_download_finished = 1;
            }
        } else {
            /* download error */
            file_write_nr = 0;
            g_app_download_finished = 1;
        }
    } else {
        // do nothing
    }

    return len;
}

/**
 * @brief Read data from flash
 *
 * @param address Address to read from
 * @param data Buffer to store read data
 * @param len Number of bytes to read
 * @return uint32_t Number of bytes read
 */
uint32_t flash_read_data (uint32_t address, uint8_t *data, uint32_t len) {
    return 0;
}

/**
 * @brief Write data to flash
 *
 * @param address Address to write to
 * @param data Buffer containing data to write
 * @param len Number of bytes to write
 * @return uint32_t Number of bytes written
 */
uint32_t flash_write_data (uint32_t address, uint8_t *data, uint32_t len) {
    if (len != VFAT16_CONF_SECTOR_SIZE) {
        memset (&data[len], 0xFF, VFAT16_CONF_SECTOR_SIZE - len);
        len = VFAT16_CONF_SECTOR_SIZE;
    }

    drv_flash_write (data, address + VFAT16_FLASH_START_ADDR, len);

    return 0;
}
