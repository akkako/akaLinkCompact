#include "ch32v30x_rcc.h"
#include "ch32v30x_usb.h"

void drv_usb_enable_enum(uint8_t en)
{
    if (en)
    {
        USBHSD->CONTROL |= USBHS_UC_DEV_PU_EN;
    }
    else
    {
        USBHSD->CONTROL &= ~USBHS_UC_DEV_PU_EN;
    }
}

void usb_dc_low_level_init(void)
{
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div3);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
    NVIC_EnableIRQ(USBHS_IRQn);
}

void usb_dc_low_level_deinit(void)
{
    drv_usb_enable_enum(0);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, DISABLE);
    NVIC_DisableIRQ(USBHS_IRQn);
}

