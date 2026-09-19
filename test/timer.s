    .globl _start
    .text

_start:
    # 1. 设置 mtvec
    la   t0, trap_handler
    csrrw zero, mtvec, t0
    li t4, 0x01
    slli t4, t4, 29

    # 2. 设置 mtimecmp = mtime + t4
    la   t0, MTIMECMP_ADDR
    la   t1, MTIME_ADDR
    lw   t2, 0(t1)
    add t2, t2, t4
    sw   t2, 0(t0)        # mtimecmp = mtime + t4

    # 3. 使能定时器中断
    li   t0, (1 << 7)     # MTIE
    csrrs zero, mie, t0   # mie |= MTIE
    
    li   t0, (1 << 3)     # MIE
    csrrs zero, mstatus, t0  # mstatus |= MIE

    # 4. 主循环：计数
    li   s0, 0
loop:
    addi s0, s0, 1        # s0++，希望执行很多次
    j    loop

# ============================================
# 中断处理程序
# ============================================
trap_handler:
    # 判断是否是中断（mcause 最高位为1）
    csrrs t0, mcause, zero
    li    t1, 0x80000000
    and   t0, t0, t1
    beqz  t0, exception   # 最高位为0，是异常

    # 是中断：输出 'T' 表示 Timer
    la    t0, UART_BASE
    li    t1, 'T'
    sb    t1, 0(t0)

    # 重新设置 mtimecmp（避免连续触发）
    la    t0, MTIMECMP_ADDR
    la    t1, MTIME_ADDR
    lw    t2, 0(t1)
    add t2, t2, t4
    sw    t2, 0(t0)

    mret

exception:
    # 异常处理（如 ECALL）
    mret
