
# SWCLK -- PB13
# SWDO  -- PB15
# SWDI  -- PB14
# SWDIR -- PB12

    .equ GPIOB_BASE, 0x40010C00

    .equ CFGLR_OS, 0x00
    .equ CFGHR_OS, 0x04
    .equ INDR_OS, 0x08
    .equ OUTDR_OS, 0x0C
    .equ BSHR_OS, 0x10
    .equ BCR_OS, 0x14

    .equ SWCLK_HIGH_MASK, 0x00002000
    .equ SWCLK_LOW_MASK, 0x20000000
    .equ SWDIO_HIGH_MASK, 0x00008000
    .equ SWDIO_LOW_MASK, 0x80000000

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
# t1 -- SWCLK_HIGH_MASK preset
# t2 -- CLK_FALLING_HIGH_MASK preset
# t3 -- CLK_FALLING_LOW_MASK preset
# t4 -- Temporary
# t5 -- Temporary
# t6 -- Temporary

SWD_Write_GPIO_Fast:
    li      t0, GPIOB_BASE                  # preload GPIOB Base address
    li      t1, SWCLK_HIGH_MASK             # clock rising and data hold preset
    li      t2, SWCLK_LOW_MASK              # clock falling and data high preset
    li      t3, SWDIO_LOW_MASK              # clock falling and data low preset

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
#=========== turnaround ===========
    sw      t2, BSHR_OS(t0)                       # generate clock falling

    li      t5, CFG_OUTPUT_MASK             # swdio config input
    sw      t5, CFGHR_OS(t0)  
    li      t5, DIR_OUTPUT_MASK             # swdir switch input
    sw      t5, BSHR_OS(t0)

    sw      t1, BSHR_OS(t0)                       # generate clock rising

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
#    andi    a5, a5, 1       # clear
# parity bit store in a5

.lable_send_data:
#=========== send data bit 0-31 ===========
.rept 32
    andi    t5, t4, 1                       # save LSB in t5
    slli    t6, t5, 4                       # t6 = t5 ? 16 : 0 
    srl     t5, t3, t6                      # t5 = t6 ? BIT15 : BIT31
    or      t5, t5, t2                      # merge 
    sw      t5, BSHR_OS(t0)                 # clock falling and data driving
    srli    t4, t4, 1                       # logic right shift for next send
    sw      t1, BSHR_OS(t0)                 # clock rising and data holding
.endr
#=========== send parity bit ===========
    andi    t5, a5, 1                       # save LSB in t5
    slli    t6, t5, 4                       # t6 = t5 ? 16 : 0 
    srl     t5, t3, t6                      # t5 = t6 ? BIT15 : BIT31
    or      t5, t5, t2                      # merge 
    sw      t5, BSHR_OS(t0)
    sw      t1, BSHR_OS(t0)                 # clock rising and data holding

.lable_ack_wait:
.lable_ack_fault:
.lable_ack_error:
    li      t4, SWDIO_HIGH_MASK
    sw      t4, BSHR_OS(t0)                       # output data high

    ret
