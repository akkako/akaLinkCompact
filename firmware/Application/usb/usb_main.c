#include "usb_main.h"
#include "drv_systick.h"
#include "app_param.h"

#define CMSIS_DAP_INTERFACE_SIZE (9 + 7 + 7)
#define CUSTOM_HID_LEN (9 + 9 + 7 + 7)

#define HIDRAW_INTERVAL 4

#define HID_CUSTOM_REPORT_DESC_SIZE 53

#define HID_INTF_NUM (0x03)

#define USBD_WINUSB_VENDOR_CODE 0x20

#define USBD_BULK_ENABLE 1
#define USBD_WINUSB_ENABLE 1

#define FUNCTION_SUBSET_LEN (160)
#define DEVICE_INTERFACE_GUIDS_FEATURE_LEN (132)

#define USBD_WINUSB_DESC_SET_LEN (WINUSB_DESCRIPTOR_SET_HEADER_SIZE + USBD_BULK_ENABLE * FUNCTION_SUBSET_LEN)

#define USBD_NUM_DEV_CAPABILITIES (USBD_WINUSB_ENABLE)

#define USBD_WINUSB_DESC_LEN (28)

#define USBD_BOS_WTOTALLENGTH (0x05 + USBD_WINUSB_DESC_LEN * USBD_WINUSB_ENABLE)

#define USB_CONFIG_SIZE (9 + CMSIS_DAP_INTERFACE_SIZE + CDC_ACM_DESCRIPTOR_LEN + CUSTOM_HID_LEN)

#define USB_PACKET_SIZE (512)

#define INTF_NUM (1 + 2 + 1)

__ALIGN_BEGIN const uint8_t USBD_WinUSBDescriptorSetDescriptor[] = {
    WBVAL(WINUSB_DESCRIPTOR_SET_HEADER_SIZE), /* wLength */
    WBVAL(WINUSB_SET_HEADER_DESCRIPTOR_TYPE), /* wDescriptorType */
    0x00, 0x00, 0x03, 0x06, /* >= Win 8.1 */  /* dwWindowsVersion*/
    WBVAL(USBD_WINUSB_DESC_SET_LEN),          /* wDescriptorSetTotalLength */
#if USBD_BULK_ENABLE
    WBVAL(WINUSB_FUNCTION_SUBSET_HEADER_SIZE), /* wLength */
    WBVAL(WINUSB_SUBSET_HEADER_FUNCTION_TYPE), /* wDescriptorType */
    0,                                         /* bFirstInterface USBD_BULK_IF_NUM*/
    0,                                         /* bReserved */
    WBVAL(FUNCTION_SUBSET_LEN),                /* wSubsetLength */
    WBVAL(WINUSB_FEATURE_COMPATIBLE_ID_SIZE),  /* wLength */
    WBVAL(WINUSB_FEATURE_COMPATIBLE_ID_TYPE),  /* wDescriptorType */
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0,        /* CompatibleId*/
    0, 0, 0, 0, 0, 0, 0, 0,                    /* SubCompatibleId*/
    WBVAL(DEVICE_INTERFACE_GUIDS_FEATURE_LEN), /* wLength */
    WBVAL(WINUSB_FEATURE_REG_PROPERTY_TYPE),   /* wDescriptorType */
    WBVAL(WINUSB_PROP_DATA_TYPE_REG_MULTI_SZ), /* wPropertyDataType */
    WBVAL(42),                                 /* wPropertyNameLength */
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0, 0,
    WBVAL(80), /* wPropertyDataLength */
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
    0x05,                         /* bLength */
    0x0f,                         /* bDescriptorType */
    WBVAL(USBD_BOS_WTOTALLENGTH), /* wTotalLength */
    USBD_NUM_DEV_CAPABILITIES,    /* bNumDeviceCaps */
#if (USBD_WINUSB_ENABLE)
    USBD_WINUSB_DESC_LEN,           /* bLength */
    0x10,                           /* bDescriptorType */
    USB_DEVICE_CAPABILITY_PLATFORM, /* bDevCapabilityType */
    0x00,                           /* bReserved */
    0xDF,
    0x60,
    0xDD,
    0xD8, /* PlatformCapabilityUUID */
    0x89,
    0x45,
    0xC7,
    0x4C,
    0x9C,
    0xD2,
    0x65,
    0x9D,
    0x9E,
    0x64,
    0x8A,
    0x9F,
    0x00,
    0x00,
    0x03,
    0x06,
    /* >= Win 8.1 */                 /* dwWindowsVersion*/
    WBVAL(USBD_WINUSB_DESC_SET_LEN), /* wDescriptorSetTotalLength */
    USBD_WINUSB_VENDOR_CODE,         /* bVendorCode */
    0,                               /* bAltEnumCode */
#endif
};

// clang-format off
#define HID_DESC()                                                                                                                   \
    /************** Descriptor of Custom interface *****************/                                                                \
    0x09,                                               /* bLength: Interface Descriptor size */                                     \
    USB_DESCRIPTOR_TYPE_INTERFACE,                  /* bDescriptorType: Interface descriptor type */                                 \
    HID_INTF_NUM,                                   /* bInterfaceNumber: Number of Interface */                                      \
    0x00,                                           /* bAlternateSetting: Alternate setting */                                       \
    0x02,                                           /* bNumEndpoints */                                                              \
    0x03,                                           /* bInterfaceClass: HID */                                                       \
    0x01,                                           /* bInterfaceSubClass : 1=BOOT, 0=no boot */                                     \
    0x00,                                           /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */                           \
    0, /* iInterface: Index of string descriptor */ /******************** Descriptor of Custom HID ********************/             \
    0x09,                                           /* bLength: HID Descriptor size */                                               \
    HID_DESCRIPTOR_TYPE_HID,                        /* bDescriptorType: HID */                                                       \
    0x11,                                           /* bcdHID: HID Class Spec release number */                                      \
    0x01,                                                                                                                            \
    0x00,                                              /* bCountryCode: Hardware target country */                                   \
    0x01,                                              /* bNumDescriptors: Number of HID class descriptors to follow */              \
    0x22,                                              /* bDescriptorType */                                                         \
    HID_CUSTOM_REPORT_DESC_SIZE,                       /* wItemLength: Total length of Report descriptor */                          \
    0x00,                                              /******************** Descriptor of Custom in endpoint ********************/  \
    0x07,                                              /* bLength: Endpoint Descriptor size */                                       \
    USB_DESCRIPTOR_TYPE_ENDPOINT,                      /* bDescriptorType: */                                                        \
    HID_IN_EP,                                         /* bEndpointAddress: Endpoint Address (IN) */                                 \
    0x03,                                              /* bmAttributes: Interrupt endpoint */                                        \
    WBVAL(HID_PACKET_SIZE),                            /* wMaxPacketSize: 4 Byte max */                                              \
    HIDRAW_INTERVAL, /* bInterval: Polling Interval */ /******************** Descriptor of Custom out endpoint ********************/ \
    0x07,                                              /* bLength: Endpoint Descriptor size */                                       \
    USB_DESCRIPTOR_TYPE_ENDPOINT,                      /* bDescriptorType: */                                                        \
    HID_OUT_EP,                                        /* bEndpointAddress: Endpoint Address (IN) */                                 \
    0x03,                                              /* bmAttributes: Interrupt endpoint */                                        \
    WBVAL(HID_PACKET_SIZE),                            /* wMaxPacketSize: 4 Byte max */                                              \
    HIDRAW_INTERVAL                                    /* bInterval: Polling Interval */
// clang-format on

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_1, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, INTF_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
    /* Endpoint IN 1 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
    /* CDC Config Macro */
    CDC_ACM_DESCRIPTOR_INIT(0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_PACKET_SIZE, 0x00),
    /* Custom HID */
    HID_DESC(),
};

static const uint8_t other_speed_config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, INTF_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
    /* Endpoint IN 1 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
    /* CDC Config Macro */
    CDC_ACM_DESCRIPTOR_INIT(0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_PACKET_SIZE, 0x00),
    /* Custom HID */
    HID_DESC(),
};

// clang-format off
/*!< custom hid report descriptor */
const uint8_t hid_custom_report_desc[HID_CUSTOM_REPORT_DESC_SIZE] = {
    /* USER CODE BEGIN 0 */
    0x06, 0x00, 0xff, /* USAGE_PAGE (Vendor Defined Page 1) */
    0x09, 0x01,       /* USAGE (Vendor Usage 1) */
    0xa1, 0x01,       /* COLLECTION (Application) */
    0x85, 0x02,       /*   REPORT ID (0x02) */
    0x09, 0x02,       /*   USAGE (Vendor Usage 1) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x25, 0xff,       /*LOGICAL_MAXIMUM (255) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x96, 0x3f, 0x00, /*   REPORT_COUNT (63) */
    0x81, 0x02,       /*   INPUT (Data,Var,Abs) */
    /* <___________________________________________________> */
    0x85, 0x01,       /*   REPORT ID (0x01) */
    0x09, 0x03,       /*   USAGE (Vendor Usage 1) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x25, 0xff,       /*   LOGICAL_MAXIMUM (255) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x96, 0x3f, 0x00, /*   REPORT_COUNT (63) */
    0x91, 0x02,       /*   OUTPUT (Data,Var,Abs) */

    /* <___________________________________________________> */
    0x85, 0x03,       /*   REPORT ID (0x03) */
    0x09, 0x04,       /*   USAGE (Vendor Usage 1) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x25, 0xff,       /*   LOGICAL_MAXIMUM (255) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x96, 0x3f, 0x00, /*   REPORT_COUNT (63) */
    0xb1, 0x02,       /*   FEATURE (Data,Var,Abs) */
    /* USER CODE END 0 */
    0xC0 /*     END_COLLECTION	             */
};
// clang-format on

char serial_number_dynamic[36] = "123456789ABCDEF"; // Dynamic serial number

char *string_descriptors[] = {
    (char[]){0x09, 0x04}, /* Langid */
    "ARM",                /* Manufacturer */
    "akaLink CMSIS-DAP",  /* Product */
};

static const uint8_t device_quality_descriptor[] = {
    USB_DEVICE_QUALIFIER_DESCRIPTOR_INIT(USB_2_1, 0x00, 0x00, 0x00, 0x01),
};

const uint8_t *device_descriptor_callback(uint8_t speed)
{
    (void)speed;
    return device_descriptor;
}

const uint8_t *config_descriptor_callback(uint8_t speed)
{
    (void)speed;
    return config_descriptor;
}

const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    (void)speed;
    return device_quality_descriptor;
}

const uint8_t *other_speed_config_descriptor_callback(uint8_t speed)
{
    (void)speed;
    return other_speed_config_descriptor;
}

const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    (void)speed;

    if (index == 3)
    {
        return serial_number_dynamic;
    }

    if (index >= (sizeof(string_descriptors) / sizeof(char *)))
    {
        return NULL;
    }
    return string_descriptors[index];
}

// WinUSB CNSIS-DAP
static volatile uint16_t USB_RequestIndexI = 0; // Request  Index In
static volatile uint16_t USB_RequestIndexO = 0; // Request  Index Out
static volatile uint16_t USB_RequestCountI = 0; // Request  Count In
static volatile uint16_t USB_RequestCountO = 0; // Request  Count Out
static volatile uint8_t USB_RequestIdle = 1;    // USB 停止接收 request 包标志

static volatile uint16_t USB_ResponseIndexI = 0; // Response Index In
static volatile uint16_t USB_ResponseIndexO = 0; // Response Index Out
static volatile uint16_t USB_ResponseCountI = 0; // Response Count In
static volatile uint16_t USB_ResponseCountO = 0; // Response Count Out
static volatile uint8_t USB_ResponseIdle = 1;    // USB 停止发送 response 包标志

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request FIFO
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE]; // Response FIFO
static uint16_t USB_RespSize[DAP_PACKET_COUNT];                                                        // Response 数据长度 FIFO

// CDC USB2UART
volatile struct cdc_line_coding g_cdc_lincoding; // 线路编码参数
volatile uint8_t config_uart = 0;                // 需要重配置串口
volatile uint8_t config_uart_transfer = 0;       // 重配置串口完成

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t uartrx_ringbuffer[CONFIG_UARTRX_RINGBUF_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t usbrx_ringbuffer[CONFIG_USBRX_RINGBUF_SIZE];
USB_NOCACHE_RAM_SECTION chry_ringbuffer_t g_uartrx; // 串口接收环形缓冲区
USB_NOCACHE_RAM_SECTION chry_ringbuffer_t g_usbrx;  // USB 接收环形缓冲区

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t usb_rx_buffer[USB_PACKET_SIZE]; // USB 接收临时缓冲区
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t usb_tx_buffer[USB_PACKET_SIZE]; // USB 发送临时缓冲区

static volatile uint8_t usbrx_full_flag = 0;
static volatile uint8_t usbtx_finished_flag = 0;
static volatile uint8_t uarttx_finished_flag = 0;

// Custom HID
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t hid_rx_buffer[HID_PACKET_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t hid_tx_buffer[HID_PACKET_SIZE];
static volatile uint8_t hid_busy_flag = 0;

void usbd_event_handler(uint8_t busid, uint8_t event)
{
    (void)busid;
    switch (event)
    {
    case USBD_EVENT_RESET:
        usbrx_full_flag = 0;
        usbtx_finished_flag = 0;
        uarttx_finished_flag = 0;
        config_uart_transfer = 0;
        hid_busy_flag = 0;
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
        /* 开始端点数据接收 */
        USB_RequestIdle = 0U;
        usbd_ep_start_read(0, DAP_OUT_EP, USB_Request[0], USB_PACKET_SIZE);
        usbd_ep_start_read(0, CDC_OUT_EP, usb_rx_buffer, USB_PACKET_SIZE);
        usbd_ep_start_read(0, HID_OUT_EP, hid_rx_buffer, HID_PACKET_SIZE);
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
}

/**
 * @brief DAP OUT 端点接收完成回调
 * @param busid 总线 ID
 * @param ep 端点号
 * @param nbytes 实际接收数据长度
 */
void dap_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;

    // 接收到 DAP_TransferAbort 指令，立即停止 DAP 传输过程
    if (USB_Request[USB_RequestIndexI][0] == ID_DAP_TransferAbort)
    {
        DAP_Data.transfer_abort = 1U;
    }
    // 将 DAP 指令存入 FIFO 中
    else
    {
        USB_RequestIndexI++;
        if (USB_RequestIndexI == DAP_PACKET_COUNT)
        {
            USB_RequestIndexI = 0U;
        }
        USB_RequestCountI++;
    }

    // DAP 指令 FIFO 未满，继续接收
    if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT)
    {
        usbd_ep_start_read(0, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
    }
    // DAP 指令 FIFO 满，停止接收
    else
    {
        USB_RequestIdle = 1U;
    }
}

/**
 * @brief DAP IN 端点发送完成回调
 * @param busid 总线 ID
 * @param ep 端点号
 * @param nbytes 实际发送数据长度
 */
void dap_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    // FIFO 不为空，开始下一次发送
    if (USB_ResponseCountI != USB_ResponseCountO)
    {
        usbd_ep_start_write(0, DAP_IN_EP, USB_Response[USB_ResponseIndexO], USB_RespSize[USB_ResponseIndexO]);
        USB_ResponseIndexO++;
        if (USB_ResponseIndexO == DAP_PACKET_COUNT)
        {
            USB_ResponseIndexO = 0U;
        }
        USB_ResponseCountO++;
    }
    // FIFO 为空，停止发送
    else
    {
        USB_ResponseIdle = 1U;
    }
}

/**
 * @brief CDC OUT 端点接收完成回调
 * @param busid 总线 ID
 * @param ep 端点号
 * @param nbytes 实际接收数据长度
 */
void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;

    // 接收数据存入环形缓冲区中
    chry_ringbuffer_write(&g_usbrx, usb_rx_buffer, nbytes);

    // 环形缓冲区未满，开始接收下一包数据
    if (chry_ringbuffer_get_free(&g_usbrx) >= DAP_PACKET_SIZE)
    {
        usbd_ep_start_read(0, CDC_OUT_EP, usb_rx_buffer, DAP_PACKET_SIZE);
    }
    else
    {
        // 环形缓冲区满，停止接收数据
        usbrx_full_flag = 1;
    }
}

/**
 * @brief CDC IN 端点发送完成回调
 * @param busid 总线 ID
 * @param ep 端点号
 * @param nbytes 实际发送数据长度
 */
void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    uint32_t size;
    uint8_t *buffer;

    // 环形缓冲区标记发送完成的数据
    chry_ringbuffer_linear_read_done(&g_uartrx, nbytes);

    // 上一包数据是满的，发送一个额外的 zlp 包
    if ((nbytes % DAP_PACKET_SIZE) == 0 && nbytes)
    {
        usbtx_finished_flag = 0;
        usbd_ep_start_write(0, CDC_IN_EP, NULL, 0);
    }
    // 正常发送数据完成
    else
    {
        // 还有剩余数据，则继续发送
        if (chry_ringbuffer_get_used(&g_uartrx))
        {
            buffer = chry_ringbuffer_linear_read_setup(&g_uartrx, &size);
            if (size > USB_PACKET_SIZE)
            {
                size = USB_PACKET_SIZE;
            }
            memcpy(usb_tx_buffer, buffer, size);
            usbtx_finished_flag = 0;
            usbd_ep_start_write(0, CDC_IN_EP, usb_tx_buffer, size);
        }
        // 发送空闲
        else
        {
            usbtx_finished_flag = 1;
        }
    }
}

void usbd_hid_custom_notify_handler(uint8_t busid, uint8_t event, void *arg)
{
    (void)busid;
    (void)event;
    (void)arg;
    //printf("hid_notify: event = %d\n", event);
}

void usbd_hid_custom_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    (void)ep;

    memset(hid_tx_buffer, 0x00, HID_PACKET_SIZE);

    app_param_proc_hid(hid_rx_buffer, hid_tx_buffer);

    hid_tx_buffer[0] = 0x02; /* IN: report id */

    usbd_ep_start_read(busid, ep, hid_rx_buffer, HID_PACKET_SIZE);
    usbd_ep_start_write(busid, HID_IN_EP, hid_tx_buffer, HID_PACKET_SIZE);
}

void usbd_hid_custom_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    (void)ep;
    (void)nbytes;

    hid_busy_flag = 0;

    // printf("actual in len:%d\r\n", (unsigned int)nbytes);
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

struct usbd_endpoint hid_custom_in_ep = {
    .ep_addr = HID_IN_EP,
    .ep_cb = usbd_hid_custom_in_callback,
};

struct usbd_endpoint hid_custom_out_ep = {
    .ep_addr = HID_OUT_EP,
    .ep_cb = usbd_hid_custom_out_callback,
};

struct usbd_interface dap_intf;
struct usbd_interface intf1;
struct usbd_interface intf2;
struct usbd_interface hid_intf;

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

void chry_dap_init(uint8_t busid, uint32_t reg_base)
{
    chry_ringbuffer_init(&g_uartrx, uartrx_ringbuffer, CONFIG_UARTRX_RINGBUF_SIZE);
    chry_ringbuffer_init(&g_usbrx, usbrx_ringbuffer, CONFIG_USBRX_RINGBUF_SIZE);

    DAP_Setup(g_param.output_mode);

    usbd_desc_register(0, &cmsisdap_descriptor);

    /*!< winusb */
    usbd_add_interface(0, &dap_intf);
    usbd_add_endpoint(0, &dap_out_ep);
    usbd_add_endpoint(0, &dap_in_ep);

    /*!< cdc acm */
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf2));
    usbd_add_endpoint(0, &cdc_out_ep);
    usbd_add_endpoint(0, &cdc_in_ep);

    /*!< hid */
    usbd_add_interface(0, usbd_hid_init_intf(0, &hid_intf, hid_custom_report_desc, HID_CUSTOM_REPORT_DESC_SIZE));
    hid_intf.notify_handler = usbd_hid_custom_notify_handler;
    usbd_add_endpoint(0, &hid_custom_in_ep);
    usbd_add_endpoint(0, &hid_custom_out_ep);

    usbd_initialize(busid, reg_base, usbd_event_handler);
}

/**
 * @brief 主循环 DAP 指令处理函数
 * @param None
 */
void chry_dap_handle(void)
{
    uint32_t n;

    // Process pending requests
    while (USB_RequestCountI != USB_RequestCountO)
    {
        // Handle Queue Commands
        n = USB_RequestIndexO;
        while (USB_Request[n][0] == ID_DAP_QueueCommands)
        {
            USB_Request[n][0] = ID_DAP_ExecuteCommands;
            n++;
            if (n == DAP_PACKET_COUNT)
            {
                n = 0U;
            }
        }

        // Execute DAP Command (process request and prepare response)
        USB_RespSize[USB_ResponseIndexI] =
            (uint16_t)DAP_ExecuteCommand(USB_Request[USB_RequestIndexO], USB_Response[USB_ResponseIndexI]);

        // Update Request Index and Count
        USB_RequestIndexO++;
        if (USB_RequestIndexO == DAP_PACKET_COUNT)
        {
            USB_RequestIndexO = 0U;
        }
        USB_RequestCountO++;

        if (USB_RequestIdle)
        {
            if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT)
            {
                USB_RequestIdle = 0U;
                usbd_ep_start_read(0, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
            }
        }

        // Update Response Index and Count
        USB_ResponseIndexI++;
        if (USB_ResponseIndexI == DAP_PACKET_COUNT)
        {
            USB_ResponseIndexI = 0U;
        }
        USB_ResponseCountI++;

        if (USB_ResponseIdle)
        {
            if (USB_ResponseCountI != USB_ResponseCountO)
            {
                // Load data from response buffer to be sent back
                n = USB_ResponseIndexO++;
                if (USB_ResponseIndexO == DAP_PACKET_COUNT)
                {
                    USB_ResponseIndexO = 0U;
                }
                USB_ResponseCountO++;
                USB_ResponseIdle = 0U;
                usbd_ep_start_write(0, DAP_IN_EP, USB_Response[n], USB_RespSize[n]);
            }
        }
    }
}

/**
 * @brief 设置线路编码回调函数
 * @param busid 总线 ID
 * @param intf 接口编号
 * @param line_coding 线路编码配置
 */
void usbd_cdc_acm_set_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    (void)busid;
    memcpy((uint8_t *)&g_cdc_lincoding, line_coding, sizeof(struct cdc_line_coding));
    config_uart = 1;
    config_uart_transfer = 0;
}

/**
 * @brief 读取线路编码回调函数
 * @param busid 总线 ID
 * @param intf 接口编号
 * @param line_coding 线路编码配置
 */
void usbd_cdc_acm_get_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    (void)busid;
    memcpy(line_coding, (uint8_t *)&g_cdc_lincoding, sizeof(struct cdc_line_coding));
}

/**
 * @brief 主循环 USB 转 UART 处理函数
 * @param None
 */
void chry_dap_usb2uart_handle(void)
{
    uint32_t size;
    uint8_t *buffer;

    // 异步执行重配置线路编码
    if (config_uart)
    {
        /* 关闭中断 */
        __disable_irq();
        config_uart = 0;
        config_uart_transfer = 1;
        /* 配置新的线路参数 */
        chry_dap_usb2uart_uart_config_callback((struct cdc_line_coding *)&g_cdc_lincoding);
        usbtx_finished_flag = 1;
        uarttx_finished_flag = 1;
        chry_ringbuffer_reset_read(&g_uartrx);
        chry_ringbuffer_reset_read(&g_usbrx);

        /* 开启中断 */
        __enable_irq();
    }

    // 未完成配置线路参数，不进行收发操作
    if (config_uart_transfer == 0)
    {
        return;
    }

    // 在主循环中检查相关空闲状态，并启动对应传输

    // 之前串口接收 FIFO 中的数据已经通过 USB 发送完成，检查是否有待发送新数据
    if (usbtx_finished_flag)
    {
        // 有新数据，启动发送
        if (chry_ringbuffer_get_used(&g_uartrx))
        {
            buffer = chry_ringbuffer_linear_read_setup(&g_uartrx, &size);
            if (size > USB_PACKET_SIZE)
            {
                size = USB_PACKET_SIZE;
            }

            usbtx_finished_flag = 0;
            memcpy(usb_tx_buffer, buffer, size);
            usbd_ep_start_write(0, CDC_IN_EP, usb_tx_buffer, size);
        }
    }

    // 之前没有串口待发送数据了，检查是否有新数据
    if (uarttx_finished_flag)
    {
        // USB 接收到了新数据，开始发送数据
        if (chry_ringbuffer_get_used(&g_usbrx))
        {
            uarttx_finished_flag = 0;
            buffer = chry_ringbuffer_linear_read_setup(&g_usbrx, &size);
            chry_dap_usb2uart_uart_send_bydma(buffer, size);
        }
    }

    // 之前 USB 接收 FIFO 已满，检查是否可以接收新数据
    if (usbrx_full_flag)
    {
        // FIFO 现在有空间，可以开始接收新数据
        if (chry_ringbuffer_get_free(&g_usbrx) >= DAP_PACKET_SIZE)
        {
            usbrx_full_flag = 0;
            usbd_ep_start_read(0, CDC_OUT_EP, usb_rx_buffer, DAP_PACKET_SIZE);
        }
    }

    drv_usb2uart_proc();
}

/* implment by user */
void chry_dap_usb2uart_uart_config_callback(struct cdc_line_coding *line_coding)
{
    if (line_coding->dwDTERate > 9000000)
    {
        line_coding->dwDTERate = 9000000;
    }
    if (line_coding->dwDTERate == 0)
    {
        return;
    }
    drv_usb2uart_set_linecoding(line_coding->dwDTERate,
                                line_coding->bCharFormat,
                                line_coding->bParityType,
                                line_coding->bDataBits);
}

/**
 * @brief 串口发送完成回调函数
 * @param size 实际发送数据长度
 */
void chry_dap_usb2uart_uart_send_complete(uint32_t size)
{
    uint8_t *buffer;

    // 标记已发送完成的数据
    chry_ringbuffer_linear_read_done(&g_usbrx, size);

    // 还有剩余待发送的数据，立即启动下一次发送
    if (chry_ringbuffer_get_used(&g_usbrx))
    {
        buffer = chry_ringbuffer_linear_read_setup(&g_usbrx, &size);
        chry_dap_usb2uart_uart_send_bydma(buffer, size);
    }
    // 没有剩余数据了，停止串口发送
    else
    {
        uarttx_finished_flag = 1;
    }
}

/* implment by user */
void chry_dap_usb2uart_uart_send_bydma(uint8_t *data, uint16_t len)
{
    drv_usb2uart_start_tx_dma(data, len);
}
