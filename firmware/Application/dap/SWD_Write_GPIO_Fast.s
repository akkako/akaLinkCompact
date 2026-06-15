
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
    .equ DATA_OUT_HIGH_MASK, 0x00008000

    .equ CFG_INPUT_MASK, 0x44334444
    .equ CFG_OUTPUT_MASK, 0x34334444

    .equ DIR_INPUT_MASK, 0x10000000
    .equ DIR_OUTPUT_MASK, 0x00001000

    .equ ACK_OK_MASK, 1
    .equ ACK_WAIT_MASK, 2
    .equ ACK_FAULT_MASK, 4


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

# Register allocate
# a0 -- header, ack return data
# a1 -- uint8_t turnaround (already zero-extended)
# a2 -- uint8_t data_phase (already zero-extended)
# a3 -- uint8_t idle_cycles (already zero-extended)
# a4 -- uint32_t *data
# a5 -- parity calculate temp
# a6 -- parity calculate temp
# a7 -- Unused
# t0 -- R32_GPIOB_BSHR address
# t1 -- CLK_RISING_ONLY_MASK preset
# t2 -- CLK_FALLING_HIGH_MASK preset
# t3 -- CLK_FALLING_LOW_MASK preset
# t4 -- Temporary
# t5 -- Temporary
# t6 -- Temporary

SWD_Write_GPIO_Fast:
    li      t0, R32_GPIOB_BSHR              # preload BSHR address
    li      t1, CLK_RISING_ONLY_MASK        # clock rising and data hold preset
    li      t2, CLK_FALLING_HIGH_MASK       # clock falling and data high preset
    li      t3, CLK_FALLING_LOW_MASK        # clock falling and data low preset

#=========== send header bit 0 ===========  # bit0 is always 1
    // andi    t5, a0, 1                       # save LSB in t5
    // bnez    t5, .header_bit0_set            # jump to .header_bitx_set
    // sw      t3, 0(t0)                       # generate clock falling and data 0
    // j       .header_bit0_end                # jump to .header_bitx_end
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

#=========== send header bit 7 ===========  # bit7 is always 1
    // andi    t5, a0, 1                       # save LSB in t5
    // bnez    t5, .header_bit7_set            # jump to .header_bitx_set
    // sw      t3, 0(t0)                       # generate clock falling and data 0
    // j       .header_bit7_end                # jump to .header_bitx_end
.header_bit7_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.header_bit7_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding

#=========== turnaround ===========

    li      t4, R32_GPIOB_CFGHR
    li      t5, CFG_INPUT_MASK
    sw      t5, 0(t4)  
    li      t5, DIR_INPUT_MASK
    sw      t5, 0(t0)

    sw      t2, 0(t0)                       # generate clock falling
    sw      t1, 0(t0)                       # generate clock rising and data sampling

#=========== sampling ack ===========
    li      a0, 0 
    li      t6, R32_GPIOB_INDR              # R32_GPIOB_INDR address preset

    sw      t2, 0(t0)                       # generate clock falling
    lw      t5, 0(t6)                       # read INDR to t5
    sw      t1, 0(t0)                       # generate clock rising
    srli    t5, t5, 14                      # move useful bit to bit 0
    andi    t5, t5, 1                       # clear other bit
    or      a0, a0, t5                      # save to a0

    sw      t2, 0(t0)                       # generate clock falling
    lw      t5, 0(t6)                       # read INDR to t5
    sw      t1, 0(t0)                       # generate clock rising
    srli    t5, t5, 13                      # move useful bit to bit 1
    andi    t5, t5, 2                       # clear other bit
    or      a0, a0, t5                      # save to a0

    sw      t2, 0(t0)                       # generate clock falling
    lw      t5, 0(t6)                       # read INDR to t5
    sw      t1, 0(t0)                       # generate clock rising
    srli    t5, t5, 12                      # move useful bit to bit 2
    andi    t5, t5, 4                       # clear other bit
    or      a0, a0, t5                      # save to a0

#=========== turnaround ===========
    sw      t2, 0(t0)                       # generate clock falling
    sw      t1, 0(t0)                       # generate clock rising

    li      t5, CFG_OUTPUT_MASK
    sw      t5, 0(t4)  
    li      t5, DIR_OUTPUT_MASK
    sw      t5, 0(t0)

#=========== check ack and branch ===========
    li      t6, ACK_OK_MASK                 # ACK_OK
    beq     a0, t6, .lable_ack_ok
    li      t6, ACK_WAIT_MASK               # ACK_WAIT
    beq     a0, t6, .lable_ack_wait
    li      t6, ACK_FAULT_MASK              # ACK_FAULT
    beq     a0, t6, .lable_ack_fault
    j       .lable_ack_error

.lable_ack_ok:
#=========== load data ===========
    lw      t4, 0(a4)      # load 32-bit data in t4
    mv      a5, t4
#=========== calc parity bit ===========
    srli    a6,a5,16
    xor     a6,a6,a5
    srli    a5,a6,8
    xor     a5,a5,a6
    srli    a6,a5,4
    xor     a6,a6,a5
    li      a5,28672
    andi    a6,a6,15
    addi    a5,a5,-1642
    sra     a5,a5,a6
    andi    a5,a5,1
# parity bit store in a5

.lable_send_data:
#=========== send data bit 0 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b0_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b0_end              # jump to .lable_data_bx_end
.lable_data_b0_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b0_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 1 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b1_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b1_end              # jump to .lable_data_bx_end
.lable_data_b1_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b1_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 2 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b2_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b2_end              # jump to .lable_data_bx_end
.lable_data_b2_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b2_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 3 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b3_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b3_end              # jump to .lable_data_bx_end
.lable_data_b3_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b3_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 4 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b4_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b4_end              # jump to .lable_data_bx_end
.lable_data_b4_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b4_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 5 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b5_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b5_end              # jump to .lable_data_bx_end
.lable_data_b5_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b5_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 6 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b6_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b6_end              # jump to .lable_data_bx_end
.lable_data_b6_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b6_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 7 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b7_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b7_end              # jump to .lable_data_bx_end
.lable_data_b7_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b7_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 1 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b8_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b8_end              # jump to .lable_data_bx_end
.lable_data_b8_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b8_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 9 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b9_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b9_end              # jump to .lable_data_bx_end
.lable_data_b9_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b9_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 10 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b10_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b10_end              # jump to .lable_data_bx_end
.lable_data_b10_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b10_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 11 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b11_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b11_end              # jump to .lable_data_bx_end
.lable_data_b11_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b11_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 12 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b12_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b12_end              # jump to .lable_data_bx_end
.lable_data_b12_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b12_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 13 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b13_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b13_end              # jump to .lable_data_bx_end
.lable_data_b13_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b13_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 14 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b14_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b14_end              # jump to .lable_data_bx_end
.lable_data_b14_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b14_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 15 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b15_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b15_end              # jump to .lable_data_bx_end
.lable_data_b15_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b15_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 16 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b16_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b16_end              # jump to .lable_data_bx_end
.lable_data_b16_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b16_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 17 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b17_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b17_end              # jump to .lable_data_bx_end
.lable_data_b17_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b17_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 18 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b18_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b18_end              # jump to .lable_data_bx_end
.lable_data_b18_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b18_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 19 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b19_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b19_end              # jump to .lable_data_bx_end
.lable_data_b19_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b19_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 20 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b20_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b20_end              # jump to .lable_data_bx_end
.lable_data_b20_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b20_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 21 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b21_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b21_end              # jump to .lable_data_bx_end
.lable_data_b21_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b21_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 22 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b22_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b22_end              # jump to .lable_data_bx_end
.lable_data_b22_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b22_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 23 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b23_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b23_end              # jump to .lable_data_bx_end
.lable_data_b23_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b23_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 24 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b24_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b24_end              # jump to .lable_data_bx_end
.lable_data_b24_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b24_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 25 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b25_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b25_end              # jump to .lable_data_bx_end
.lable_data_b25_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b25_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 26 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b26_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b26_end              # jump to .lable_data_bx_end
.lable_data_b26_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b26_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 27 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b27_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b27_end              # jump to .lable_data_bx_end
.lable_data_b27_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b27_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 28 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b28_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b28_end              # jump to .lable_data_bx_end
.lable_data_b28_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b28_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 29 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b29_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b29_end              # jump to .lable_data_bx_end
.lable_data_b29_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b29_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 30 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b30_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b30_end              # jump to .lable_data_bx_end
.lable_data_b30_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b30_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding
    srli    t4, t4, 1                       # logic right shift for next send

#=========== send data bit 31 ===========
    andi    t5, t4, 1                       # save LSB in t5
    bnez    t5, .lable_data_b31_set          # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_data_b31_end              # jump to .lable_data_bx_end
.lable_data_b31_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_data_b31_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding

#=========== send parity bit ===========
    bnez    a5, .lable_parity_set           # jump to .lable_data_bx_set
    sw      t3, 0(t0)                       # generate clock falling and data 0
    j       .lable_parity_end              # jump to .lable_data_bx_end
.lable_parity_set:
    sw      t2, 0(t0)                       # generate clock falling and data 1
.lable_parity_end:
    sw      t1, 0(t0)                       # generate clock rising and data holding


.lable_ack_wait:
.lable_ack_fault:
.lable_ack_error:

    li      t1, DATA_OUT_HIGH_MASK
    sw      t1, 0(t0)                       # output data high

    ret
