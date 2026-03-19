/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/

#include "DAP_config.h"
#include "DAP.h"
#include "drv_gpio.h"

static inline uint8_t GetParity(uint32_t data)
{
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
    PIN_SWDIO_OUT(bit);   \
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

#define PIN_DELAY() PIN_DELAY_SLOW (DAP_Data.clock_delay)
// #define PIN_DELAY() PIN_DELAY_FAST()

// Generate SWJ Sequence
//   count:  sequence bit count
//   data:   pointer to sequence bit data
//   return: none
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
void SWJ_Sequence(uint32_t count, const uint8_t *data)
{
    uint32_t val;
    uint32_t n;

    val = 0U;
    n = 0U;
    while (count--)
    {
        if (n == 0U)
        {
            val = *data++;
            n = 8U;
        }
        if (val & 1U)
        {
            PIN_SWDIO_TMS_SET();
        }
        else
        {
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
void SWD_Sequence(uint32_t info, const uint8_t *swdo, uint8_t *swdi)
{
    uint32_t val;
    uint32_t bit;
    uint32_t n, k;

    n = info & SWD_SEQUENCE_CLK;
    if (n == 0U)
    {
        n = 64U;
    }

    if (info & SWD_SEQUENCE_DIN)
    {
        while (n)
        {
            val = 0U;
            for (k = 8U; k && n; k--, n--)
            {
                SW_READ_BIT(bit);
                val >>= 1;
                val |= bit << 7;
            }
            val >>= k;
            *swdi++ = (uint8_t)val;
        }
    }
    else
    {
        while (n)
        {
            val = *swdo++;
            for (k = 8U; k && n; k--, n--)
            {
                SW_WRITE_BIT(val);
                val >>= 1;
            }
        }
    }
}
#endif

#if (DAP_SWD != 0)

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

    /* Write 8 bit header */
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

// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t SWD_Transfer(uint32_t request, uint32_t *data)
{
    if (DAP_Data.fast_clock)
    {
        return SWD_Transfer_GPIO(request, data);
    }
    else
    {
        return SWD_Transfer_GPIO(request, data);
    }
}

#endif /* (DAP_SWD != 0) */
