#include "drv_bkp.h"
#include "ch32v30x_bkp.h"
#include "ch32v30x_pwr.h"
#include "ch32v30x_rcc.h"

void drv_bkp_init (void) {
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    BKP_TamperPinCmd (DISABLE);
    PWR_BackupAccessCmd (ENABLE);
    BKP_ClearFlag();
}

void drv_bkp_write_reg (uint16_t data) {
    BKP_WriteBackupRegister (BKP_DR1, data);
}

uint16_t drv_bkp_read_reg (void) {
    return BKP_ReadBackupRegister (BKP_DR1);
}