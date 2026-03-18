
#include "ch32v30x_misc.h"

#include "drv_print.h"
#include "drv_systick.h"
#include <stdio.h>

void chry_dap_init(uint8_t busid, uint32_t reg_base);
void chry_dap_handle(void);
void chry_dap_usb2uart_handle(void);

int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);
    drv_print_init();
    drv_systick_init();

	chry_dap_init(0, 0);
    printf("System Init\r\n");

	while(1)
    {
    // printf("System Init\r\n");
		chry_dap_handle();
        chry_dap_usb2uart_handle();
	}
    return 0;
}
