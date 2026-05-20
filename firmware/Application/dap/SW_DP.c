#include "DAP_Port.h"
#include "DAP.h"
#include "drv_spi.h"

static inline uint8_t GetParity (uint32_t data) {
    data ^= data >> 16;
    data ^= data >> 8;
    data ^= data >> 4;
    data &= 0x0F;
    return (0x6996 >> data) & 1;
}

// SW Macros

#define PIN_SWCLK_SET PIN_SWCLK_TCK_SET
#define PIN_SWCLK_CLR PIN_SWCLK_TCK_CLR

#define SW_CLOCK_CYCLE() \
    PIN_SWCLK_CLR();     \
    PIN_DELAY();         \
    PIN_SWCLK_SET();     \
    PIN_DELAY()

#define SW_WRITE_BIT(bit) \
    PIN_SWDIO_OUT (bit);  \
    PIN_SWCLK_CLR();      \
    PIN_DELAY();          \
    PIN_SWCLK_SET();      \
    PIN_DELAY()

#define SW_READ_BIT(bit)  \
    PIN_SWCLK_CLR();      \
    PIN_DELAY();          \
    bit = PIN_SWDIO_IN(); \
    PIN_SWCLK_SET();      \
    PIN_DELAY()

#define SW_READ_BIT_OPT(bit) \
    PIN_SWCLK_CLR();         \
    bit = PIN_SWDIO_IN();    \
    PIN_SWCLK_SET();         \
    PIN_DELAY();

// #define PIN_DELAY() PIN_DELAY_SLOW (DAP_Data.clock_delay)
#define PIN_DELAY() PIN_DELAY_FAST()

// Generate SWJ Sequence
//   count:  sequence bit count
//   data:   pointer to sequence bit data
//   return: none
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
void SWJ_Sequence (uint32_t count, const uint8_t *data) {
    uint32_t val;
    uint32_t n;
    // uint8_t dummy[64] = {0};

    // drv_spi_gpio_mux_spi();
    // drv_spi_dma_transmit ((uint8_t *)&data, (uint8_t *)&dummy, count / 8);
    // drv_spi_dma_wait();
    // drv_spi_gpio_mux_gpio_out();


    val = 0U;
    n = 0U;
    while (count--) {
        if (n == 0U) {
            val = *data++;
            n = 8U;
        }
        if (val & 1U) {
            PIN_SWDIO_TMS_SET();
        } else {
            PIN_SWDIO_TMS_CLR();
        }
        SW_CLOCK_CYCLE();
        val >>= 1;
        n--;
    }
}
#endif

// Generate SWD Sequence
//   info:   sequence information
//   swdo:   pointer to SWDIO generated data
//   swdi:   pointer to SWDIO captured data
//   return: none
#if (DAP_SWD != 0)
void SWD_Sequence (uint32_t info, const uint8_t *swdo, uint8_t *swdi) {
    uint32_t val;
    uint32_t bit;
    uint32_t n, k;


    n = info & SWD_SEQUENCE_CLK;
    if (n == 0U) {
        n = 64U;
    }

    if (info & SWD_SEQUENCE_DIN) {
        while (n) {
            val = 0U;
            for (k = 8U; k && n; k--, n--) {
                SW_READ_BIT (bit);
                val >>= 1;
                val |= bit << 7;
            }
            val >>= k;
            *swdi++ = (uint8_t)val;
        }
    } else {
        while (n) {
            val = *swdo++;
            for (k = 8U; k && n; k--, n--) {
                SW_WRITE_BIT (val);
                val >>= 1;
            }
        }
    }
}
#endif

#if (DAP_SWD != 0)

#undef PIN_DELAY()
#define PIN_DELAY() PIN_DELAY_FAST()

#if 0
// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
static uint8_t SWD_Transfer_GPIO(uint32_t request, uint32_t *data)
{
    uint32_t ack;
    uint32_t bit;
    uint32_t val;
    uint32_t parity;
    uint8_t header;
    uint32_t n;

    parity = (request >> 0) + (request >> 1) + (request >> 2) + (request >> 3);
    header = (0x01)                    /* Start Bit */
             | ((request & 0x0F) << 1) /* APnDP, RnW, A2, A3 Bit */
             | ((parity & 0x01) << 5)  /* Parity Bit */
             | (0 << 6)                /* Stop Bit */
             | (1 << 7) /* Park Bit */;

    /* 发送 8 bit 包头 */
    for (uint8_t n = 8U; n; n--)
    {
        SW_WRITE_BIT(header);
        header >>= 1;
    }

    /* Turnaround */
    PIN_SWDIO_OUT_DISABLE();
    for (n = DAP_Data.swd_conf.turnaround; n; n--)
    {
        SW_CLOCK_CYCLE();
    }

    /* Acknowledge response */
    SW_READ_BIT(bit);
    ack = bit << 0;
    SW_READ_BIT(bit);
    ack |= bit << 1;
    SW_READ_BIT(bit);
    ack |= bit << 2;

    if (ack == DAP_TRANSFER_OK)
    { /* OK response */
        /* Data transfer */
        if (request & DAP_TRANSFER_RnW)
        {
            /* Read data */
            val = 0U;
            for (n = 32U; n; n--)
            {
                SW_READ_BIT(bit); /* Read RDATA[0:31] */
                val >>= 1;
                val |= bit << 31;
            }
            SW_READ_BIT(bit); /* Read Parity */
            parity = GetParity(val);
            if ((parity ^ bit) & 1U)
            {
                ack = DAP_TRANSFER_ERROR;
            }
            if (data)
            {
                *data = val;
            }
            /* Turnaround */
            for (n = DAP_Data.swd_conf.turnaround; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            PIN_SWDIO_OUT_ENABLE();
        }
        else
        {
            /* Turnaround */
            for (n = DAP_Data.swd_conf.turnaround; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            PIN_SWDIO_OUT_ENABLE();
            /* Write data */
            val = *data;
            parity = GetParity(val);
            for (n = 32U; n; n--)
            {
                SW_WRITE_BIT(val); /* Write WDATA[0:31] */
                val >>= 1;
            }
            SW_WRITE_BIT(parity); /* Write Parity Bit */
        }
        /* Capture Timestamp */
        if (request & DAP_TRANSFER_TIMESTAMP)
        {
            DAP_Data.timestamp = TIMESTAMP_GET();
        }
        /* Idle cycles */
        n = DAP_Data.transfer.idle_cycles;
        if (n)
        {
            PIN_SWDIO_OUT(0U);
            for (; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
        }
        PIN_SWDIO_OUT(1U);
        return ((uint8_t)ack);
    }

    if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))
    {
        /* WAIT or FAULT response */
        if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U))
        {
            for (n = 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Dummy Read RDATA[0:31] + Parity */
            }
        }
        /* Turnaround */
        for (n = DAP_Data.swd_conf.turnaround; n; n--)
        {
            SW_CLOCK_CYCLE();
        }
        PIN_SWDIO_OUT_ENABLE();
        if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U))
        {
            PIN_SWDIO_OUT(0U);
            for (n = 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Dummy Write WDATA[0:31] + Parity */
            }
        }
        PIN_SWDIO_OUT(1U);
        return ((uint8_t)ack);
    }

    /* Protocol error */
    for (n = DAP_Data.swd_conf.turnaround + 32U + 1U; n; n--)
    {
        SW_CLOCK_CYCLE(); /* Back off data phase */
    }
    PIN_SWDIO_OUT_ENABLE();
    PIN_SWDIO_OUT(1U);
    return ((uint8_t)ack);
}

static uint8_t SWD_Transfer_SPI(uint8_t request, uint32_t *data)
{
    register uint32_t ack;
    register uint32_t bit;
    uint32_t val;
    register uint8_t parity;
    register uint8_t turn = DAP_Data.swd_conf.turnaround;
    register uint8_t header = 0;
    uint32_t dummy = 0xFFFFFFFF;

    parity = (request >> 0) + (request >> 1) + (request >> 2) + (request >> 3);

    header = (0x01)                    /* Start Bit */
             | ((request & 0x0F) << 1) /* APnDP, RnW, A2, A3 Bit */
             | ((parity & 0x01) << 5)  /* Parity Bit */
             | (0 << 6)                /* Stop Bit */
             | (1 << 7) /* Park Bit */;

    // 读流程
    if (request & DAP_TRANSFER_RnW)
    {
        /* 发送 8 bit 包头 */
        drv_spi_gpio_mux_spi();
        drv_spi_tx(header);
        drv_spi_gpio_mux_gpio();

        /* 方向转换 */
        PIN_SWDIO_OUT_DISABLE();
        for (uint8_t n = turn; n; n--)
        {
            SW_CLOCK_CYCLE();
        }

        /* 读取目标 ACK */
        SW_READ_BIT(bit);
        ack = bit << 0;
        SW_READ_BIT(bit);
        ack |= bit << 1;
        SW_READ_BIT(bit);
        ack |= bit << 2;

        if (ack == DAP_TRANSFER_OK)
        {
            /* 读数据 */
            val = 0U;

            drv_spi_gpio_mux_spi();
            drv_spi_dma_transmit((uint8_t *)&dummy, (uint8_t *)&val, 4);
            drv_spi_dma_wait();
            drv_spi_gpio_mux_gpio();

            /* 读校验位 */
            SW_READ_BIT(bit);

            parity = GetParity(val);

            if ((parity ^ bit) & 1U)
            {
                ack = DAP_TRANSFER_ERROR;
            }
            *data = val;

            /* 方向调转 */
            for (uint8_t n = turn; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            PIN_SWDIO_OUT_ENABLE();

            /* 传输空闲时钟 */
            uint8_t n = DAP_Data.transfer.idle_cycles;
            if (n)
            {
                PIN_SWDIO_OUT(0U);
                for (; n; n--)
                {
                    SW_CLOCK_CYCLE();
                }
            }

            /* SWDIO 切换输出 */
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }

        // 回复 WAIT 或者 FAULT
        else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))
        {

            /* WAIT or FAULT response */
            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U))
            {
                for (uint8_t n = 32U + 1U; n; n--)
                {
                    SW_CLOCK_CYCLE(); /* Dummy Read RDATA[0:31] + Parity */
                }
            }
            /* Turnaround */
            for (uint8_t n = turn; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            PIN_SWDIO_OUT_ENABLE();
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }
        else
        {
            /* Protocol error */
            for (uint8_t n = turn + 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Back off data phase */
            }
            PIN_SWDIO_OUT_ENABLE();
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }
    }
    // 写流程
    else
    {
        /* 发送 8 bit 包头 */
        drv_spi_gpio_mux_spi();
        drv_spi_tx(header);
        drv_spi_gpio_mux_gpio();

        /* 方向转换 */
        PIN_SWDIO_OUT_DISABLE();
        for (uint8_t n = turn; n; n--)
        {
            SW_CLOCK_CYCLE();
        }

        /* 读取目标 ACK */
        SW_READ_BIT(bit);
        ack = bit << 0;
        SW_READ_BIT(bit);
        ack |= bit << 1;
        SW_READ_BIT(bit);
        ack |= bit << 2;

        if (ack == DAP_TRANSFER_OK)
        {
            /* 方向调转 */
            for (uint8_t n = turn; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            // val = *data;

            PIN_SWDIO_OUT_ENABLE();
            parity = GetParity(*data);

            /* 写 32 位数据 */
            drv_spi_gpio_mux_spi();
            drv_spi_dma_transmit((uint8_t *)data, (uint8_t *)&dummy, 4);
            drv_spi_dma_wait();
            drv_spi_gpio_mux_gpio();
            /* 写校验位 */
            SW_WRITE_BIT(parity);

            /* 传输空闲时钟 */
            uint8_t n = DAP_Data.transfer.idle_cycles;
            if (n)
            {
                PIN_SWDIO_OUT(0U);
                for (; n; n--)
                {
                    SW_CLOCK_CYCLE();
                }
            }

            /* SWDIO 切换输出 */
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }
        else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))
        {
            /* WAIT or FAULT response */
            for (uint8_t n = turn; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
            PIN_SWDIO_OUT_ENABLE();
            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U))
            {
                PIN_SWDIO_OUT(0U);
                for (uint8_t n = 32U + 1U; n; n--)
                {
                    SW_CLOCK_CYCLE(); /* Dummy Write WDATA[0:31] + Parity */
                }
            }
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }
        else
        {
            /* Protocol error */
            for (uint8_t n = turn + 32U + 1U; n; n--)
            {
                SW_CLOCK_CYCLE(); /* Back off data phase */
            }
            PIN_SWDIO_OUT_ENABLE();
            PIN_SWDIO_OUT(1U);
            return ((uint8_t)ack);
        }
    }
}

// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t SWD_Transfer(uint32_t request, uint32_t *data)
{
    if (DAP_Data.fast_clock)
    {
        return SWD_Transfer_SPI(request, data);
        // return SWD_Transfer_GPIO (request, data);
    }
    else
    {
        return SWD_Transfer_SPI(request, data);
        // return SWD_Transfer_GPIO (request, data);
    }
}
#endif

static uint8_t SWD_Read_SPI (uint8_t request, uint32_t *data) {
    register uint32_t ack = 0;
    register uint32_t ack1 = 0;
    register uint32_t ack2 = 0;
    register uint8_t parity;
    register uint8_t turn = DAP_Data.swd_conf.turnaround;
    register uint8_t header = 0;

    uint32_t val = 0;
    uint32_t dummy = 0xFFFFFFFF;

    parity = (request >> 0) + (request >> 1) + (request >> 2) + (request >> 3);

    header = (0x01)                    /* Start Bit */
             | ((request & 0x0F) << 1) /* APnDP, RnW, A2, A3 Bit */
             | ((parity & 0x01) << 5)  /* Parity Bit */
             | (0 << 6)                /* Stop Bit */
             | (1 << 7);               /* Park Bit */

    /* 发送 8 bit 包头 */
    drv_spi_gpio_mux_spi();
    drv_spi_tx (header);
    drv_spi_gpio_mux_gpio_in();

    /* 方向转换 */
    PIN_SWDIO_OUT_DISABLE();
    for (uint8_t n = turn; n; n--) {
        SW_CLOCK_CYCLE();
    }

    /* 读取目标 ACK */
    SW_READ_BIT_OPT (ack);
    SW_READ_BIT_OPT (ack1);
    SW_READ_BIT_OPT (ack2);
    ack = (ack2 << 2) | (ack1 << 1) | (ack);

    if (ack == DAP_TRANSFER_OK) {
        /* 读数据 */
        drv_spi_gpio_mux_spi();
        drv_spi_dma_transmit ((uint8_t *)&dummy, (uint8_t *)&val, 4);
        drv_spi_dma_wait();
        drv_spi_gpio_mux_gpio_in();

        parity = GetParity (val);

        /* 读校验位 */
        SW_READ_BIT (ack1);

        if ((parity ^ ack1) & 1U) {
            ack = DAP_TRANSFER_ERROR;
        }
        *data = val;

        /* 方向调转 */
        for (uint8_t n = turn; n; n--) {
            SW_CLOCK_CYCLE();
        }
        PIN_SWDIO_OUT_ENABLE();

        /* 传输空闲时钟 */
        uint8_t n = DAP_Data.transfer.idle_cycles;
        if (n) {
            PIN_SWDIO_OUT (0U);
            for (; n; n--) {
                SW_CLOCK_CYCLE();
            }
        }

        /* SWDIO 切换输出 */
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    }

    // 回复 WAIT 或者 FAULT
    else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {

        /* WAIT or FAULT response */
        if (DAP_Data.swd_conf.data_phase) {
            for (uint8_t n = 32U + 1U; n; n--) {
                SW_CLOCK_CYCLE(); /* Dummy Read RDATA[0:31] + Parity */
            }
        }
        /* Turnaround */
        for (uint8_t n = turn; n; n--) {
            SW_CLOCK_CYCLE();
        }
        PIN_SWDIO_OUT_ENABLE();
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    } else {
        /* Protocol error */
        for (uint8_t n = turn + 32U + 1U; n; n--) {
            SW_CLOCK_CYCLE(); /* Back off data phase */
        }
        PIN_SWDIO_OUT_ENABLE();
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    }

    return ((uint8_t)ack);
}

static uint8_t SWD_Write_SPI (uint8_t request, uint32_t *data) {
    register uint32_t ack;
    register uint32_t ack1 = 0;
    register uint32_t ack2 = 0;
    register uint8_t parity;
    register uint8_t turn = DAP_Data.swd_conf.turnaround;
    register uint8_t header = 0;

    uint32_t val = *data;
    uint32_t dummy = 0xFFFFFFFF;

    parity = (request >> 0) + (request >> 1) + (request >> 2) + (request >> 3);

    header = (0x01)                    /* Start Bit */
             | ((request & 0x0F) << 1) /* APnDP, RnW, A2, A3 Bit */
             | ((parity & 0x01) << 5)  /* Parity Bit */
             | (0 << 6)                /* Stop Bit */
             | (1 << 7);               /* Park Bit */

    /* 发送 8 bit 包头 */
    drv_spi_gpio_mux_spi();
    drv_spi_tx (header);
    drv_spi_gpio_mux_gpio_in();

    /* 方向转换 */
    PIN_SWDIO_OUT_DISABLE();
    for (uint8_t n = turn; n; n--) {
        SW_CLOCK_CYCLE();
    }

    /* 读取目标 ACK */
    SW_READ_BIT_OPT (ack);
    SW_READ_BIT_OPT (ack1);
    SW_READ_BIT_OPT (ack2);
    ack = (ack2 << 2) | (ack1 << 1) | (ack);


    if (ack == DAP_TRANSFER_OK) {
        /* 方向调转 */
        for (uint8_t n = turn; n; n--) {
            SW_CLOCK_CYCLE();
        }

        PIN_SWDIO_OUT_ENABLE();

        /* 写 32 位数据 */
        drv_spi_gpio_mux_spi();
        drv_spi_dma_transmit ((uint8_t *)&val, (uint8_t *)&dummy, 4);
        parity = GetParity (val);
        drv_spi_dma_wait();
        drv_spi_gpio_mux_gpio_out();
        /* 写校验位 */
        SW_WRITE_BIT (parity);

        /* 传输空闲时钟 */
        uint8_t n = DAP_Data.transfer.idle_cycles;
        if (n) {
            PIN_SWDIO_OUT (0U);
            for (; n; n--) {
                SW_CLOCK_CYCLE();
            }
        }

        /* SWDIO 切换输出 */
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    } else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {
        /* WAIT or FAULT response */
        for (uint8_t n = turn; n; n--) {
            SW_CLOCK_CYCLE();
        }
        PIN_SWDIO_OUT_ENABLE();
        if (DAP_Data.swd_conf.data_phase) {
            PIN_SWDIO_OUT (0U);
            for (uint8_t n = 32U + 1U; n; n--) {
                SW_CLOCK_CYCLE(); /* Dummy Write WDATA[0:31] + Parity */
            }
        }
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    } else {
        /* Protocol error */
        for (uint8_t n = turn + 32U + 1U; n; n--) {
            SW_CLOCK_CYCLE(); /* Back off data phase */
        }
        PIN_SWDIO_OUT_ENABLE();
        PIN_SWDIO_OUT (1U);
        return ((uint8_t)ack);
    }
}

uint8_t SWD_Read (uint32_t request, uint32_t *data) {
    return SWD_Read_SPI (request, data);
}

uint8_t SWD_Write (uint32_t request, uint32_t *data) {
    return SWD_Write_SPI (request, data);
}


#endif /* (DAP_SWD != 0) */
