#include "usb_main.h"

#define CMSIS_DAP_INTERFACE_SIZE (9 + 7 + 7)

#define USBD_WINUSB_VENDOR_CODE 0x20
#define USBD_WEBUSB_VENDOR_CODE 0x21

#define USBD_BULK_ENABLE 1
#define USBD_WINUSB_ENABLE 1

/* WinUSB Microsoft OS 2.0 descriptor sizes */
#define WINUSB_DESCRIPTOR_SET_HEADER_SIZE 10
#define WINUSB_FUNCTION_SUBSET_HEADER_SIZE 8
#define WINUSB_FEATURE_COMPATIBLE_ID_SIZE 20

#define FUNCTION_SUBSET_LEN 160
#define DEVICE_INTERFACE_GUIDS_FEATURE_LEN 132

#define USBD_WINUSB_DESC_SET_LEN (WINUSB_DESCRIPTOR_SET_HEADER_SIZE + USBD_BULK_ENABLE * FUNCTION_SUBSET_LEN)

#define USBD_NUM_DEV_CAPABILITIES (USBD_WINUSB_ENABLE)

#define USBD_WINUSB_DESC_LEN 28

#define USBD_BOS_WTOTALLENGTH (0x05 + USBD_WINUSB_DESC_LEN * USBD_WINUSB_ENABLE)

#define USB_CONFIG_SIZE (9 + CMSIS_DAP_INTERFACE_SIZE + CDC_ACM_DESCRIPTOR_LEN)

#define INTF_NUM (1 + 2)

#define MSC_INTF_NUM (3)

#define WEBUSB_INTF_NUM (3)

#define WEBUSB_URL_STRINGS \
    'c', 'h', 'e', 'r', 'r', 'y', 'd', 'a', 'p', '.', 'c', 'h', 'e', 'r', 'r', 'y', '-', 'e', 'm', 'b', 'e', 'd', 'd', 'e', 'd', '.', 'o', 'r', 'g',

__ALIGN_BEGIN const uint8_t USBD_WinUSBDescriptorSetDescriptor[] = {
    WBVAL (WINUSB_DESCRIPTOR_SET_HEADER_SIZE),   /* wLength */
    WBVAL (WINUSB_SET_HEADER_DESCRIPTOR_TYPE),   /* wDescriptorType */
    0x00, 0x00, 0x03, 0x06, /* >= Win 8.1 */     /* dwWindowsVersion*/
    WBVAL (USBD_WINUSB_DESC_SET_LEN),            /* wDescriptorSetTotalLength */
#if USBD_BULK_ENABLE
    WBVAL (WINUSB_FUNCTION_SUBSET_HEADER_SIZE), /* wLength */
    WBVAL (WINUSB_SUBSET_HEADER_FUNCTION_TYPE), /* wDescriptorType */
    0,                                          /* bFirstInterface USBD_BULK_IF_NUM*/
    0,                                          /* bReserved */
    WBVAL (FUNCTION_SUBSET_LEN),                /* wSubsetLength */
    WBVAL (WINUSB_FEATURE_COMPATIBLE_ID_SIZE),  /* wLength */
    WBVAL (WINUSB_FEATURE_COMPATIBLE_ID_TYPE),  /* wDescriptorType */
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0,         /* CompatibleId*/
    0, 0, 0, 0, 0, 0, 0, 0,                     /* SubCompatibleId*/
    WBVAL (DEVICE_INTERFACE_GUIDS_FEATURE_LEN), /* wLength */
    WBVAL (WINUSB_FEATURE_REG_PROPERTY_TYPE),   /* wDescriptorType */
    WBVAL (WINUSB_PROP_DATA_TYPE_REG_MULTI_SZ), /* wPropertyDataType */
    WBVAL (42),                                 /* wPropertyNameLength */
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0, 0,
    WBVAL (80), /* wPropertyDataLength */
    '{', 0,
    'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0, 'A', 0, 'D', 0, '-', 0,
    '2', 0, '9', 0, '3', 0, 'B', 0, '-', 0,
    '4', 0, '6', 0, '6', 0, '3', 0, '-', 0,
    'A', 0, 'A', 0, '3', 0, '6', 0, '-',
    0, '1', 0, 'A', 0, 'A', 0, 'E', 0, '4', 0, '6', 0, '4', 0, '6', 0, '3', 0, '7', 0, '7', 0, '6', 0,
    '}', 0, 0, 0, 0, 0
#endif
};

__ALIGN_BEGIN const uint8_t USBD_BinaryObjectStoreDescriptor[] = {
    0x05,                           /* bLength */
    0x0f,                           /* bDescriptorType */
    WBVAL (USBD_BOS_WTOTALLENGTH),  /* wTotalLength */
    USBD_NUM_DEV_CAPABILITIES,      /* bNumDeviceCaps */
#if (USBD_WINUSB_ENABLE)
    USBD_WINUSB_DESC_LEN,           /* bLength */
    0x10,                           /* bDescriptorType */
    USB_DEVICE_CAPABILITY_PLATFORM, /* bDevCapabilityType */
    0x00,                           /* bReserved */
    0xDF, 0x60, 0xDD, 0xD8,         /* PlatformCapabilityUUID */
    0x89, 0x45, 0xC7, 0x4C,
    0x9C, 0xD2, 0x65, 0x9D,
    0x9E, 0x64, 0x8A, 0x9F,
    0x00, 0x00, 0x03, 0x06, /* >= Win 8.1 */ /* dwWindowsVersion*/
    WBVAL (USBD_WINUSB_DESC_SET_LEN),        /* wDescriptorSetTotalLength */
    USBD_WINUSB_VENDOR_CODE,                 /* bVendorCode */
    0,                                       /* bAltEnumCode */
#endif
};

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT (USB_2_1, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT (USB_CONFIG_SIZE, INTF_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT (0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT (DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    /* Endpoint IN 1 */
    USB_ENDPOINT_DESCRIPTOR_INIT (DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    CDC_ACM_DESCRIPTOR_INIT (0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, DAP_PACKET_SIZE, 0x00),
#if USBD_WEBUSB_ENABLE
    USB_INTERFACE_DESCRIPTOR_INIT (WEBUSB_INTF_NUM, 0x00, 0x00, 0xff, 0x00, 0x00, 0x04),
#endif
};

static const uint8_t other_speed_config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT (USB_CONFIG_SIZE, INTF_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT (0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT (DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    /* Endpoint IN 1 */
    USB_ENDPOINT_DESCRIPTOR_INIT (DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    CDC_ACM_DESCRIPTOR_INIT (0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, DAP_PACKET_SIZE, 0x00),
#if USBD_WEBUSB_ENABLE
    USB_INTERFACE_DESCRIPTOR_INIT (WEBUSB_INTF_NUM, 0x00, 0x00, 0xff, 0x00, 0x00, 0x04),
#endif
};

char serial_number_dynamic[36] = "123456789ABCDEF";  // Dynamic serial number

char *string_descriptors[] = {
    (char[]){0x09, 0x04}, /* Langid */
    "CherryUSB", /* Manufacturer */
    "CherryUSB CMSIS-DAP", /* Product */
    "123456789ABCDEF", /* Serial Number */
};

static const uint8_t device_quality_descriptor[] = {
    USB_DEVICE_QUALIFIER_DESCRIPTOR_INIT (USB_2_1, 0x00, 0x00, 0x00, 0x01),
};

__WEAK const uint8_t *device_descriptor_callback (uint8_t speed) {
    (void)speed;
    return device_descriptor;
}

__WEAK const uint8_t *config_descriptor_callback (uint8_t speed) {
    (void)speed;
    return config_descriptor;
}

__WEAK const uint8_t *device_quality_descriptor_callback (uint8_t speed) {
    (void)speed;
    return device_quality_descriptor;
}

__WEAK const uint8_t *other_speed_config_descriptor_callback (uint8_t speed) {
    (void)speed;
    return other_speed_config_descriptor;
}

__WEAK const char *string_descriptor_callback (uint8_t speed, uint8_t index) {
    (void)speed;

    if (index == 3) {
        return serial_number_dynamic;
    }

    if (index >= (sizeof (string_descriptors) / sizeof (char *))) {
        return NULL;
    }
    return string_descriptors[index];
}

// WinUSB CNSIS-DAP
static volatile uint16_t USB_RequestIndexI = 0;                                                         // Request  Index In
static volatile uint16_t USB_RequestIndexO = 0;                                                         // Request  Index Out
static volatile uint16_t USB_RequestCountI = 0;                                                         // Request  Count In
static volatile uint16_t USB_RequestCountO = 0;                                                         // Request  Count Out
static volatile uint8_t USB_RequestIdle = 1;                                                            // Request  Idle  Flag

static volatile uint16_t USB_ResponseIndexI = 0;                                                        // Response Index In
static volatile uint16_t USB_ResponseIndexO = 0;                                                        // Response Index Out
static volatile uint16_t USB_ResponseCountI = 0;                                                        // Response Count In
static volatile uint16_t USB_ResponseCountO = 0;                                                        // Response Count Out
static volatile uint8_t USB_ResponseIdle = 1;                                                           // Response Idle  Flag

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];   // Request  Buffer
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer
static uint16_t USB_RespSize[DAP_PACKET_COUNT];                                                         // Response Size

// CDC USB2UART
volatile struct cdc_line_coding g_cdc_lincoding;
volatile uint8_t config_uart = 0;
volatile uint8_t config_uart_transfer = 0;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t uartrx_ringbuffer[CONFIG_UARTRX_RINGBUF_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t usbrx_ringbuffer[CONFIG_USBRX_RINGBUF_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t usb_tmpbuffer[DAP_PACKET_SIZE];

static volatile uint8_t usbrx_full_flag = 0;
static volatile uint8_t usbtx_empty_flag = 0;
static volatile uint8_t uarttx_empty_flag = 0;

USB_NOCACHE_RAM_SECTION chry_ringbuffer_t g_uartrx;
USB_NOCACHE_RAM_SECTION chry_ringbuffer_t g_usbrx;

void usbd_event_handler (uint8_t busid, uint8_t event) {
    (void)busid;
    switch (event) {
    case USBD_EVENT_RESET:
        usbrx_full_flag = 0;
        usbtx_empty_flag = 0;
        uarttx_empty_flag = 0;
        config_uart_transfer = 0;
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
        /* setup first out ep read transfer */
        USB_RequestIdle = 0U;

        usbd_ep_start_read (0, DAP_OUT_EP, USB_Request[0], DAP_PACKET_SIZE);
        usbd_ep_start_read (0, CDC_OUT_EP, usb_tmpbuffer, DAP_PACKET_SIZE);

        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
}

void dap_out_callback (uint8_t busid, uint8_t ep, uint32_t nbytes) {
    (void)busid;
    if (USB_Request[USB_RequestIndexI][0] == ID_DAP_TransferAbort) {
        DAP_Data.transfer_abort = 1U;
    } else {
        USB_RequestIndexI++;
        if (USB_RequestIndexI == DAP_PACKET_COUNT) {
            USB_RequestIndexI = 0U;
        }
        USB_RequestCountI++;
    }

    // Start reception of next request packet
    if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT) {
        usbd_ep_start_read (0, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
    } else {
        USB_RequestIdle = 1U;
    }
}

void dap_in_callback (uint8_t busid, uint8_t ep, uint32_t nbytes) {
    (void)busid;
    if (USB_ResponseCountI != USB_ResponseCountO) {
        // Load data from response buffer to be sent back
        usbd_ep_start_write (0, DAP_IN_EP, USB_Response[USB_ResponseIndexO], USB_RespSize[USB_ResponseIndexO]);
        USB_ResponseIndexO++;
        if (USB_ResponseIndexO == DAP_PACKET_COUNT) {
            USB_ResponseIndexO = 0U;
        }
        USB_ResponseCountO++;
    } else {
        USB_ResponseIdle = 1U;
    }
}

/**
 * @brief Callback when usb receive new data.
 *
 * @param busid
 * @param ep
 * @param nbytes
 */
void usbd_cdc_acm_bulk_out (uint8_t busid, uint8_t ep, uint32_t nbytes) {
    (void)busid;

    // printf("usb rx data fin: %u\r\n", nbytes);

    // receive usb data, store into ringbuffer
    chry_ringbuffer_write (&g_usbrx, usb_tmpbuffer, nbytes);

    // if ringbuffer is not full, save more data into ringbuffer
    if (chry_ringbuffer_get_free (&g_usbrx) >= DAP_PACKET_SIZE) {
        usbd_ep_start_read (0, CDC_OUT_EP, usb_tmpbuffer, DAP_PACKET_SIZE);
    } else {
        // usbrx ringbuffer has no space to store data
        usbrx_full_flag = 1;
    }
}

/**
 * @brief Callback when usb send data finished.
 *
 * @param busid
 * @param ep
 * @param nbytes
 */
void usbd_cdc_acm_bulk_in (uint8_t busid, uint8_t ep, uint32_t nbytes) {
    (void)busid;
    uint32_t size;
    uint8_t *buffer;

    printf ("usb tx data fin: %u\r\n", nbytes);

    // clear the data already has been sent
    chry_ringbuffer_linear_read_done (&g_uartrx, nbytes);

    // previous packet was full, send another zlp indicate ending.
    if ((nbytes % DAP_PACKET_SIZE) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write (0, CDC_IN_EP, NULL, 0);
    } else {
        // normal send finished, if has more data to send, then continue.
        if (chry_ringbuffer_get_used (&g_uartrx)) {
            // get a linear area
            buffer = chry_ringbuffer_linear_read_setup (&g_uartrx, &size);
            // send data
            usbd_ep_start_write (0, CDC_IN_EP, buffer, size);
        } else {
            usbtx_empty_flag = 1;
        }
    }
}

struct usbd_endpoint dap_out_ep = {
    .ep_addr = DAP_OUT_EP,
    .ep_cb = dap_out_callback};

struct usbd_endpoint dap_in_ep = {
    .ep_addr = DAP_IN_EP,
    .ep_cb = dap_in_callback};

struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in};

struct usbd_interface dap_intf;
struct usbd_interface intf1;
struct usbd_interface intf2;

struct usb_msosv2_descriptor msosv2_desc = {
    .vendor_code = USBD_WINUSB_VENDOR_CODE,
    .compat_id = USBD_WinUSBDescriptorSetDescriptor,
    .compat_id_len = USBD_WINUSB_DESC_SET_LEN,
};

struct usb_bos_descriptor bos_desc = {
    .string = USBD_BinaryObjectStoreDescriptor,
    .string_len = USBD_BOS_WTOTALLENGTH};

const struct usb_descriptor cmsisdap_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .other_speed_descriptor_callback = other_speed_config_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback,
    .bos_descriptor = &bos_desc,
    .msosv2_descriptor = &msosv2_desc};

void chry_dap_init (uint8_t busid, uint32_t reg_base) {
    chry_ringbuffer_init (&g_uartrx, uartrx_ringbuffer, CONFIG_UARTRX_RINGBUF_SIZE);
    chry_ringbuffer_init (&g_usbrx, usbrx_ringbuffer, CONFIG_USBRX_RINGBUF_SIZE);

    DAP_Setup();

    usbd_desc_register (0, &cmsisdap_descriptor);

    /*!< winusb */
    usbd_add_interface (0, &dap_intf);
    usbd_add_endpoint (0, &dap_out_ep);
    usbd_add_endpoint (0, &dap_in_ep);

    /*!< cdc acm */
    usbd_add_interface (0, usbd_cdc_acm_init_intf (0, &intf1));
    usbd_add_interface (0, usbd_cdc_acm_init_intf (0, &intf2));
    usbd_add_endpoint (0, &cdc_out_ep);
    usbd_add_endpoint (0, &cdc_in_ep);

    usbd_initialize (busid, reg_base, usbd_event_handler);
}

void chry_dap_handle (void) {
    uint32_t n;

    // Process pending requests
    while (USB_RequestCountI != USB_RequestCountO) {
        // Handle Queue Commands
        n = USB_RequestIndexO;
        while (USB_Request[n][0] == ID_DAP_QueueCommands) {
            USB_Request[n][0] = ID_DAP_ExecuteCommands;
            n++;
            if (n == DAP_PACKET_COUNT) {
                n = 0U;
            }
            if (n == USB_RequestIndexI) {
                // flags = osThreadFlagsWait(0x81U, osFlagsWaitAny, osWaitForever);
                // if (flags & 0x80U) {
                //     break;
                // }
            }
        }

        // Execute DAP Command (process request and prepare response)
        USB_RespSize[USB_ResponseIndexI] =
            (uint16_t)DAP_ExecuteCommand (USB_Request[USB_RequestIndexO], USB_Response[USB_ResponseIndexI]);

        // Update Request Index and Count
        USB_RequestIndexO++;
        if (USB_RequestIndexO == DAP_PACKET_COUNT) {
            USB_RequestIndexO = 0U;
        }
        USB_RequestCountO++;

        if (USB_RequestIdle) {
            if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT) {
                USB_RequestIdle = 0U;
                usbd_ep_start_read (0, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
            }
        }

        // Update Response Index and Count
        USB_ResponseIndexI++;
        if (USB_ResponseIndexI == DAP_PACKET_COUNT) {
            USB_ResponseIndexI = 0U;
        }
        USB_ResponseCountI++;

        if (USB_ResponseIdle) {
            if (USB_ResponseCountI != USB_ResponseCountO) {
                // Load data from response buffer to be sent back
                n = USB_ResponseIndexO++;
                if (USB_ResponseIndexO == DAP_PACKET_COUNT) {
                    USB_ResponseIndexO = 0U;
                }
                USB_ResponseCountO++;
                USB_ResponseIdle = 0U;
                usbd_ep_start_write (0, DAP_IN_EP, USB_Response[n], USB_RespSize[n]);
            }
        }
    }
}

void usbd_cdc_acm_set_line_coding (uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding) {
    (void)busid;
    if (memcmp (line_coding, (uint8_t *)&g_cdc_lincoding, sizeof (struct cdc_line_coding)) != 0) {
        memcpy ((uint8_t *)&g_cdc_lincoding, line_coding, sizeof (struct cdc_line_coding));
        config_uart = 1;
        config_uart_transfer = 0;
    }
}

void usbd_cdc_acm_get_line_coding (uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding) {
    (void)busid;
    memcpy (line_coding, (uint8_t *)&g_cdc_lincoding, sizeof (struct cdc_line_coding));
}

void chry_dap_usb2uart_handle (void) {
    uint32_t size;
    uint8_t *buffer;

    // need config uart parameter
    if (config_uart) {
        /* disable irq here */
        __disable_irq();
        config_uart = 0;
        /* config uart here */
        chry_dap_usb2uart_uart_config_callback ((struct cdc_line_coding *)&g_cdc_lincoding);
        usbtx_empty_flag = 1;
        uarttx_empty_flag = 1;
        config_uart_transfer = 1;
        chry_ringbuffer_reset_read (&g_uartrx);
        chry_ringbuffer_reset_read (&g_usbrx);
        /* enable irq here */
        __enable_irq();
    }

    if (config_uart_transfer == 0) {
        return;
    }

    /* check whether usb tx ringbuffer is empty */
    if (usbtx_empty_flag) {
        // has data, need start usb send
        if (chry_ringbuffer_get_used (&g_uartrx)) {
            usbtx_empty_flag = 0;
            /* start first transfer */
            buffer = chry_ringbuffer_linear_read_setup (&g_uartrx, &size);
            usbd_ep_start_write (0, CDC_IN_EP, buffer, size);
            printf ("usb tx start: %u\r\n", size);
        }
    }

    /* check whether uart tx ringbuffer is empty */
    if (uarttx_empty_flag) {
        // usb rx ringbuffer has data, need start uart send
        if (chry_ringbuffer_get_used (&g_usbrx)) {
            uarttx_empty_flag = 0;
            /* start first transfer */
            buffer = chry_ringbuffer_linear_read_setup (&g_usbrx, &size);
            chry_dap_usb2uart_uart_send_bydma (buffer, size);
            // printf("uart tx start: %u\r\n", size);
        }
    }

    /* check whether usb rx ringbuffer have space to store */
    if (usbrx_full_flag) {
        // usb rx ringbuffer has space, then start to read more
        if (chry_ringbuffer_get_free (&g_usbrx) >= DAP_PACKET_SIZE) {
            usbrx_full_flag = 0;
            usbd_ep_start_read (0, CDC_OUT_EP, usb_tmpbuffer, DAP_PACKET_SIZE);
        }
    }
}

/* implment by user */
// __WEAK void chry_dap_usb2uart_uart_config_callback(struct cdc_line_coding *line_coding)
// {
// }

/* called by user */
void chry_dap_usb2uart_uart_send_complete (uint32_t size) {
    uint8_t *buffer;

    chry_ringbuffer_linear_read_done (&g_usbrx, size);

    if (chry_ringbuffer_get_used (&g_usbrx)) {
        buffer = chry_ringbuffer_linear_read_setup (&g_usbrx, &size);
        chry_dap_usb2uart_uart_send_bydma (buffer, size);
    } else {
        // printf("uart tx fin\r\n");
        uarttx_empty_flag = 1;
    }
}

/* implment by user */
// __WEAK void chry_dap_usb2uart_uart_send_bydma(uint8_t *data, uint16_t len)
// {
// }
