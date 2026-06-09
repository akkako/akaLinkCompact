
#include "usbd_core.h"
#include "usbd_msc.h"
#include "flash_fat16.h"

/*!< endpoint address */
#define MSC_IN_EP 0x81
#define MSC_OUT_EP 0x02

#define USBD_VID 0x0D28
#define USBD_PID 0x0204
#define USBD_MAX_POWER 250
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + MSC_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#ifdef CONFIG_USB_HS
#define MSC_MAX_MPS 512
#else
#define MSC_MAX_MPS 64
#endif

#ifdef CONFIG_USBDEV_ADVANCE_DESC
static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT (USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01)};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT (USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT (0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02)};

static const uint8_t device_quality_descriptor[] = {
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
};

char dynamic_serial_number[25] = {0};

static const char *string_descriptors[] = {
    (const char[]){0x09, 0x04}, /* Langid */
    "ARM", /* Manufacturer */
    "akaLinkDFU", /* Product */
};

static const uint8_t *device_descriptor_callback (uint8_t speed) {
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback (uint8_t speed) {
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback (uint8_t speed) {
    return device_quality_descriptor;
}

static const char *string_descriptor_callback (uint8_t speed, uint8_t index) {
    if (index > 3) {
        return NULL;
    }
    if (index == 3) {
        return dynamic_serial_number;
    }
    return string_descriptors[index];
}

const struct usb_descriptor msc_ram_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback};
#else

uint8_t msc_ram_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT (USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT (USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT (0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT (USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x08,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'A', 0x00,                  /* wcChar0 */
    'R', 0x00,                  /* wcChar1 */
    'M', 0x00,                  /* wcChar2 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'a', 0x00,                  /* wcChar0 */
    'k', 0x00,                  /* wcChar1 */
    'a', 0x00,                  /* wcChar2 */
    'L', 0x00,                  /* wcChar3 */
    'i', 0x00,                  /* wcChar4 */
    'n', 0x00,                  /* wcChar5 */
    'k', 0x00,                  /* wcChar6 */
    'D', 0x00,                  /* wcChar7 */
    'F', 0x00,                  /* wcChar8 */
    'U', 0x00,                  /* wcChar9 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x32,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '1', 0x00,                  /* wcChar0 */
    '2', 0x00,                  /* wcChar1 */
    '3', 0x00,                  /* wcChar2 */
    '4', 0x00,                  /* wcChar3 */
    '5', 0x00,                  /* wcChar4 */
    '6', 0x00,                  /* wcChar5 */
    '7', 0x00,                  /* wcChar6 */
    '8', 0x00,                  /* wcChar7 */
    '9', 0x00,                  /* wcChar8 */
    '0', 0x00,                  /* wcChar9 */
    'A', 0x00,                  /* wcChar10 */
    'B', 0x00,                  /* wcChar11 */
    '1', 0x00,                  /* wcChar12 */
    '2', 0x00,                  /* wcChar13 */
    '3', 0x00,                  /* wcChar14 */
    '4', 0x00,                  /* wcChar15 */
    '5', 0x00,                  /* wcChar16 */
    '6', 0x00,                  /* wcChar17 */
    '7', 0x00,                  /* wcChar18 */
    '8', 0x00,                  /* wcChar19 */
    '9', 0x00,                  /* wcChar20 */
    '0', 0x00,                  /* wcChar21 */
    'A', 0x00,                  /* wcChar22 */
    'B', 0x00,                  /* wcChar23 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
#endif
    0x00};
#endif

static void usbd_event_handler (uint8_t busid, uint8_t event) {
    switch (event) {
    case USBD_EVENT_RESET:
        break;
    case USBD_EVENT_CONNECTED:
        break;
    case USBD_EVENT_DISCONNECTED:
        break;
    case USBD_EVENT_RESUME:
        break;
    case USBD_EVENT_SUSPEND:
        break;
    case USBD_EVENT_CONFIGURED:
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
}

struct usbd_interface intf0;

static const char hex_format[] = "0123456789ABCDEF";

void usb_msc_init (uint8_t busid, uintptr_t reg_base) {
#ifdef CONFIG_USBDEV_ADVANCE_DESC
    // Fix UID
    for (int j = 0; j < 3; j++) {
        uint32_t IC_UID = *(uint32_t *)(0x1FFFF7E8 + j * 4);
        for (int i = 0; i < 8; i++) {
            dynamic_serial_number[i + j * 8] = hex_format[(IC_UID >> i * 4) & 0xF];
        }
    }
    usbd_desc_register (busid, &msc_ram_descriptor);
#else
    uint8_t *p_sn = &msc_ram_descriptor[145 - 59];
    // Fix UID
    for (int j = 0; j < 3; j++) {
        uint32_t IC_UID = *(uint32_t *)(0x1FFFF7E8 + j * 4);
        for (int i = 0; i < 8; i++) {
            p_sn[(i + j * 8) * 2] = hex_format[(IC_UID >> i * 4) & 0xF];
        }
    }
    usbd_desc_register (busid, msc_ram_descriptor);
#endif
    
    usbd_add_interface (busid, usbd_msc_init_intf (busid, &intf0, MSC_OUT_EP, MSC_IN_EP));
    usbd_initialize (busid, reg_base, usbd_event_handler);
}

void usbd_msc_get_cap (uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size) {
    *block_num = 64;
    *block_size = VFAT16_CONF_SECTOR_SIZE;
}

int usbd_msc_sector_read (uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length) {
    flash_fat16_read (sector * VFAT16_CONF_SECTOR_SIZE, buffer, length);
    return 0;
}

int usbd_msc_sector_write (uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length) {
    flash_fat16_write (sector * VFAT16_CONF_SECTOR_SIZE, buffer, length);
    return 0;
}
