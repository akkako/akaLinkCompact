
# SWCLK -- PB13
# SWDO  -- PB15
# SWDI  -- PB14
# SWDIR -- PB12

    .equ GPIOB_BASE,        0x40010C00

    .equ CFGLR_OS,          0x00
    .equ CFGHR_OS,          0x04
    .equ INDR_OS,           0x08
    .equ OUTDR_OS,          0x0C
    .equ BSHR_OS,           0x10
    .equ BCR_OS,            0x14

    .equ SWCLK_HIGH_MASK,   0x00002000
    .equ SWCLK_LOW_MASK,    0x20000000
    .equ SWDIO_HIGH_MASK,   0x00008000
    .equ SWDIO_LOW_MASK,    0x80000000

    .equ CFG_INPUT_MASK,    0x44334444
    .equ CFG_OUTPUT_MASK,   0x34334444

    .equ DIR_INPUT_MASK,    0x10000000
    .equ DIR_OUTPUT_MASK,   0x00001000

    .equ ACK_OK_MASK,       0x01
    .equ ACK_WAIT_MASK,     0x02
    .equ ACK_FAULT_MASK,    0x04
    .equ ACK_ERROR_MASK,    0x08

    .text
    .align  2
    .global SWD_Read_GPIO_Fast
    .type   SWD_Read_GPIO_Fast, @function

# definition
# uint8_t SWD_Read_GPIO_Fast(uint8_t header, uint8_t turnaround, uint8_t data_phase, uint8_t idle_cycles, uint32_t *data)
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
# a5 -- Temporary
# a6 -- Temporary
# a7 -- Temporary
# t0 -- GPIOB_BASE address
# t1 -- SWCLK_HIGH_MASK preset
# t2 -- CLK_FALLING_HIGH_MASK preset
# t3 -- CLK_FALLING_LOW_MASK preset
# t4 -- Temporary
# t5 -- Temporary
# t6 -- Temporary

SWD_Read_GPIO_Fast:
    li      t0, GPIOB_BASE
    li      t1, SWCLK_HIGH_MASK
    li      t2, SWCLK_LOW_MASK
    li      t3, SWDIO_LOW_MASK

#=========== send header bits ===========
.rept 8
    andi    t4, a0, 1                       # save LSB in t4
    slli    t5, t4, 4                       # t5 = t4 ? 16 : 0 
    srl     t4, t3, t5                      # t4 = t5 ? BIT15 : BIT31
    or      t4, t4, t2                      # merge 
    sw      t4, BSHR_OS(t0)
    srli    a0, a0, 1                       # logic right shift for next send
    sw      t1, BSHR_OS(t0)                 # clock rising and data holding
.endr
#=========== turnaround ===========
    li      t4, CFG_INPUT_MASK
    sw      t4, CFGHR_OS(t0)  
    li      t4, DIR_INPUT_MASK
    sw      t4, BSHR_OS(t0)

    sw      t2, BSHR_OS(t0)                 # generate clock falling
    sw      t1, BSHR_OS(t0)                 # generate clock rising and data sampling
#=========== sampling ack ===========
    // li      a0, 0 

    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      t4, INDR_OS(t0)                 # read INDR to t4
    sw      t1, BSHR_OS(t0)                 # generate clock rising
    srli    t4, t4, 14                      # move useful bit to bit 0
    andi    t4, t4, 1                       # clear other bit
    or      a0, a0, t4                      # save to a0

    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      t4, INDR_OS(t0)                 # read INDR to t4
    sw      t1, BSHR_OS(t0)                 # generate clock rising
    srli    t4, t4, 13                      # move useful bit to bit 1
    andi    t4, t4, 2                       # clear other bit
    or      a0, a0, t4                      # save to a0

    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      t4, INDR_OS(t0)                 # read INDR to t4
    sw      t1, BSHR_OS(t0)                 # generate clock rising
    srli    t4, t4, 12                      # move useful bit to bit 2
    andi    t4, t4, 4                       # clear other bit
    or      a0, a0, t4                      # save to a0

#=========== check ack and branch ===========
    li      t4, ACK_OK_MASK                 # ACK_OK
    beq     a0, t4, .lable_ack_ok
    li      t4, ACK_WAIT_MASK               # ACK_WAIT
    beq     a0, t4, .lable_ack_wait
    li      t4, ACK_FAULT_MASK              # ACK_FAULT
    beq     a0, t4, .lable_ack_fault
    j       .lable_ack_error

.lable_ack_ok:
#=========== read data bit 0-31 ===========
    li      t4, 0x80000000
    mv      t6, x0

# read bit 0 in a5
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      a5, INDR_OS(t0)                 # read INDR to a5
    sw      t1, BSHR_OS(t0)                 # generate clock rising
.rept 15
# read bit 1(29) in a6
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      a6, INDR_OS(t0)                 # read INDR to a6
    sw      t1, BSHR_OS(t0)                 # generate clock rising
# process bit 0(28) in a5 to t6
    slli    a5, a5, 17                      # move useful bit to bit 31
    and     a5, a5, t4                      # clear other bit
    or      t6, t6, a5                      # save to t6
    srli    t6, t6, 1                       # shift
# read bit 2(30) in a5
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      a5, INDR_OS(t0)                 # read INDR to a5
    sw      t1, BSHR_OS(t0)                 # generate clock rising
# process bit 1(29) in a6 to t6
    slli    a6, a6, 17                      # move useful bit to bit 31
    and     a6, a6, t4                      # clear other bit
    or      t6, t6, a6                      # save to t6
    srli    t6, t6, 1                       # shift
.endr

# read bit 31 in a6
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      a6, INDR_OS(t0)                 # read INDR to a6
    sw      t1, BSHR_OS(t0)                 # generate clock rising
# process bit 30 in a5 to t6
    slli    a5, a5, 17                      # move useful bit to bit 31
    and     a5, a5, t4                      # clear other bit
    or      t6, t6, a5                      # save to t6
    srli    t6, t6, 1                       # shift
# read parity in a5
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    lw      a5, INDR_OS(t0)                 # read INDR to a5
    sw      t1, BSHR_OS(t0)                 # generate clock rising
# process bit 31 in a6 to t6
    slli    a6, a6, 17                      # move useful bit to bit 31
    and     a6, a6, t4                      # clear other bit
    or      t6, t6, a6                      # save to t6
# process parity bit in a5 to t4
    srli    a5, a5, 14                      # move useful bit to bit 0
    andi    t4, a5, 1                       # clear other bit

#=========== calc parity bit  ===========
    mv      a5, t4
    srli    a6, a5, 16
    xor     a5, a5, a6      # 32 -- 16
    srli    a6, a5, 8
    xor     a5, a5, a6      # 16 -- 8
    srli    a6, a5, 4
    xor     a5, a5, a6      # 8 -- 4
    srli    a6, a5, 2
    xor     a5, a5, a6      # 4 -- 2
    srli    a6, a5, 1
    xor     a5, a5, a6      # 2 -- 1
    // andi    a5, a5, 1       # clear other bit

    xor     t4, t4, a5
    andi    t4, t4, 1       # clear other bit
    beq     t4, x0, .L_parity_check_ok
    li      a0, ACK_ERROR_MASK

.L_parity_check_ok:
    sw      t6, 0(a4)       # save data in memory

#=========== turnaround ===========

    li      t5, CFG_OUTPUT_MASK             # swdio config input
    sw      t5, CFGHR_OS(t0)  
    li      t5, DIR_OUTPUT_MASK             # swdir switch input
    sw      t5, BSHR_OS(t0)

    sw      t2, BSHR_OS(t0)                       # generate clock falling
    sw      t1, BSHR_OS(t0)                       # generate clock rising
    ret

.lable_ack_wait:
.lable_ack_fault:
#=========== turnaround ===========
    li      t5, CFG_OUTPUT_MASK             # swdio config input
    sw      t5, CFGHR_OS(t0)  
    li      t5, DIR_OUTPUT_MASK             # swdir switch input
    sw      t5, BSHR_OS(t0)

    sw      t2, BSHR_OS(t0)                       # generate clock falling
    sw      t1, BSHR_OS(t0)                       # generate clock rising
    ret

.lable_ack_error:
#=========== empty read 33 bit ===========
.rept 33
    sw      t2, BSHR_OS(t0)                 # generate clock falling
    sw      t1, BSHR_OS(t0)                 # generate clock rising
.endr
#=========== turnaround ===========
    li      t5, CFG_OUTPUT_MASK             # swdio config input
    sw      t5, CFGHR_OS(t0)  
    li      t5, DIR_OUTPUT_MASK             # swdir switch input
    sw      t5, BSHR_OS(t0)

    sw      t2, BSHR_OS(t0)                       # generate clock falling
    sw      t1, BSHR_OS(t0)                       # generate clock rising
    ret