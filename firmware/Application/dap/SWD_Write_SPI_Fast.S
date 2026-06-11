# =============================================================================
#  常量定义
# =============================================================================
    .equ GPIOB_BASE,      0x40010C00
    .equ SPI2_BASE,       0x40003800
    .equ DMA1_CH5_BASE,   0x40020058
    .equ DAP_DATA_BASE,   0x20005A68

    # GPIO 寄存器偏移
    .equ GPIO_CFGHR,      0x04
    .equ GPIO_INDR,       0x08
    .equ GPIO_BSHR,       0x10
    .equ GPIO_BCR,        0x14

    # SPI 寄存器偏移
    .equ SPI_STATR,       0x08
    .equ SPI_DATAR,       0x0C

    # DMA 寄存器偏移
    .equ DMA_CFGR,        0x00
    .equ DMA_CNTR,        0x04
    .equ DMA_MADDR,       0x0C

    # DAP_Data 偏移
    .equ DAP_TURNAROUND,  0x18
    .equ DAP_DATA_PHASE,  0x19
    .equ DAP_IDLE_CYCLES, 0x0C

    # 引脚掩码（根据你的硬件修改）
    .equ PIN_SWCLK,       0x2000        # PB13
    .equ PIN_SWDIR,       0x1000        # PB12
    .equ PIN_SWDIO_IN,    0x4000        # PB14（输入采样）
    .equ PIN_SWDIO_OUT,   0x8000        # PB15（输出驱动）

    # CFGHR 配置值
    .equ MUX_SPI,         0xB8B38B44
    .equ MUX_GPIO_IN,     0x48338B44
    .equ MUX_GPIO_OUT,    0x38338B44

# =============================================================================
#  操作宏（保持与 C 宏同名，可读性最佳）
# =============================================================================
    .macro MUX_SPI_MODE
        lui   t0, 0xB8B39
        addi  t0, t0, -1212          # t0 = MUX_SPI
        sw    t0, GPIO_CFGHR(s0)
    .endm

    .macro MUX_GPIO_IN_MODE
        lui   t0, 0x48339
        addi  t0, t0, -1212          # t0 = MUX_GPIO_IN
        sw    t0, GPIO_CFGHR(s0)
    .endm

    .macro MUX_GPIO_OUT_MODE
        lui   t0, 0x38339
        addi  t0, t0, -1212          # t0 = MUX_GPIO_OUT
        sw    t0, GPIO_CFGHR(s0)
    .endm

    .macro PIN_SWCLK_TCK_CLR
        li    t0, PIN_SWCLK
        sw    t0, GPIO_BCR(s0)
    .endm

    .macro PIN_SWCLK_TCK_SET
        li    t0, PIN_SWCLK
        sw    t0, GPIO_BSHR(s0)
    .endm

    .macro PIN_SWDIR_INPUT
        li    t0, PIN_SWDIR
        sw    t0, GPIO_BCR(s0)       # PB12 = 0 -> 输入方向
    .endm

    .macro PIN_SWDIR_OUTPUT
        li    t0, PIN_SWDIR
        sw    t0, GPIO_BSHR(s0)      # PB12 = 1 -> 输出方向
    .endm

    .macro PIN_SWDIO_OUT_0
        li    t0, PIN_SWDIO_OUT
        sw    t0, GPIO_BCR(s0)
    .endm

    .macro PIN_SWDIO_OUT_1
        li    t0, PIN_SWDIO_OUT
        sw    t0, GPIO_BSHR(s0)
    .endm

    .macro SPI_WAIT_BUSY
    1:      lhu   t0, SPI_STATR(s1)
            andi  t0, t0, 0x80       # BSY 标志
            bnez  t0, 1b
    .endm

    .macro SW_CLOCK_CYCLE
            PIN_SWCLK_TCK_CLR
            PIN_SWCLK_TCK_SET
    .endm

    # 优化版 ACK 位读取：与反汇编逻辑一致
    # 输出：目标寄存器 \rd = 0/1
    .macro SW_READ_BIT_OPT rd
            li    t0, PIN_SWCLK
            sw    t0, GPIO_BCR(s0)    # SWCLK 低
            lw    \rd, GPIO_INDR(s0)   # 读 INDR
            sw    t0, GPIO_BSHR(s0)   # SWCLK 高
            srli  \rd, \rd, 14         # 提取 PB14
            andi  \rd, \rd, 1
    .endm

    .globl  SWD_Write_SPI_Fast
    .type   SWD_Write_SPI_Fast, @function
SWD_Write_SPI_Fast:
    # --- 保存 callee-saved 寄存器 ---
    addi    sp, sp, -24
    sw      s0,  0(sp)
    sw      s1,  4(sp)
    sw      s2,  8(sp)
    sw      s3,  12(sp)
    sw      s4,  16(sp)
    sw      ra,  20(sp)             # 防御性保存，尽管无子程序调用

    # --- 一次性加载外设基址（整个函数内不再 lui）---
    lui     s0, 0x40011
    addi    s0, s0, -1024           # s0 = GPIOB_BASE (0x40010C00)

    lui     s1, 0x40004
    addi    s1, s1, -2048           # s1 = SPI2_BASE (0x40003800)

    lui     s2, 0x40020
    addi    s2, s2, 0x58            # s2 = DMA1_CH5_BASE (0x40020058)

    lui     s3, 0x20006
    addi    s3, s3, -1432           # s3 = DAP_DATA (0x20005A68)

    # a0 = header, a1 = data pointer