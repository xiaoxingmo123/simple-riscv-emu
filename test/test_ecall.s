    .globl _start
    .text

_start:
    la   sp, _stack_top      # 栈顶 = 8MB
    la   a0, _uart_base      # a0 = 0x10000000

    # 清零 .bss
    la   t0, _bss_start
    la   t1, _bss_end
_clear_bss:
    bge  t0, t1, _bss_done
    sb   zero, 0(t0)
    addi t0, t0, 1
    j    _clear_bss
_bss_done:

    # 你的代码...
    call main
    ebreak

main:
    la t0, handler
    csrrw zero, mtvec, t0

    ecall
    li t1, 0x1010  # won't be executed
handler:
    csrrs t2, mepc, zero
    la t0, end
    csrrw zero, mepc, t0
    mret

end:
    li s1, 1
    ebreak

    .data
msg:
    .string "Hello\n"
