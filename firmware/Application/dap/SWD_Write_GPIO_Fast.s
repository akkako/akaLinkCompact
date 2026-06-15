
    .equ R32_GPIOB_CFGLR, 0x40010C00
    .equ R32_GPIOB_CFGHR, 0x40010C04
    .equ R32_GPIOB_INDR, 0x40010C08
    .equ R32_GPIOB_OUTDR, 0x40010C0C
    .equ R32_GPIOB_BSHR, 0x40010C10
    .equ R32_GPIOB_BCR, 0x40010C14

# SWCLK -- PB13
# SWDO  -- PB15
# SWDI  -- PB14
# SWDIR -- PB12

    .equ CLK_RISING_ONLY_MASK, 0x00002000
    .equ CLK_FALLING_HIGH_MASK, 0x20008000
    .equ CLK_FALLING_LOW_MASK, 0xA0000000



    .text
    .align  2
    .global SWD_Write_GPIO_Fast
    .type   SWD_Write_GPIO_Fast, @function

# definition
# uint8_t SWD_Write_GPIO_Fast(uint8_t header, uint8_t turnaround, uint8_t data_phase, uint8_t idle_cycles, uint32_t *data)
#
# parameter:
# a0 -- uint8_t header (already zero-extended)
# a1 -- uint8_t turnaround (already zero-extended)
# a2 -- uint8_t data_phase (already zero-extended)
# a3 -- uint8_t idle_cycles (already zero-extended)
# a4 -- uint32_t *data
#
# return value:
# a0 -- ack (zero-extend to 32-bit)

SWD_Write_GPIO_Fast:
    li      t0, R32_GPIOB_BSHR              # preload BSHR address
    li      t1, CLK_RISING_ONLY_MASK        # clock rising and data hold preset
    li      t2, CLK_FALLING_HIGH_MASK       # clock falling and data high preset
    li      t3, CLK_FALLING_LOW_MASK        # clock falling and data low preset
    
#=========== send header bit 0 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit0_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit0_end                # jump to .header_bitx_end
.header_bit0_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit0_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 1 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit1_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit1_end                # jump to .header_bitx_end
.header_bit1_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit1_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 2 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit2_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit2_end                # jump to .header_bitx_end
.header_bit2_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit2_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 3 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit3_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit3_end                # jump to .header_bitx_end
.header_bit3_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit3_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 4 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit4_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit4_end                # jump to .header_bitx_end
.header_bit4_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit4_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 5 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit5_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit5_end                # jump to .header_bitx_end
.header_bit5_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit5_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 6 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit6_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit6_end                # jump to .header_bitx_end
.header_bit6_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit6_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    a0, a0, 1                       # logic right shift for next send

#=========== send header bit 7 ===========
    andi    t5, a0, 1                       # save LSB in t5
    bnez    t5, .header_bit7_set            # jump to .header_bitx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .header_bit7_end                # jump to .header_bitx_end
.header_bit7_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit7_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding



#    lw      t0, 0(a1)      # load 32-bit data in t0

    li      a0, 0         # 返回值放入 a0
    ret                    # 等价于 jalr x0, ra, 0