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
    # 设置 mtvec = trap_handler
    la t0, trap_handler
    csrrw zero, mtvec, t0

    li a7, 64
    li a0, 1
    la a1, msg
    li a2, 6
    ecall

    li a7, 93
    li a0, 0
    ecall

    ebreak # 这里不会执行

trap_handler:
    li t0, 64
    beq a7, t0, sys_write
    li t0, 93
    beq a7, t0, sys_exit

    j trap_done

sys_write:
    la t0, _uart_base
    mv t1, a1
    mv t2, a2
    mv t3, zero
_write_loop:
    bge t3, t2, _write_done
    add t4, t1, t3
    lbu t5, 0(t4)
    sb t5, 0(t0)
    addi t3, t3, 1
    j _write_loop
_write_done:
    j trap_done

sys_exit:
    li t0, 1
    sw t0, exit_flag, t1
    j trap_done

trap_done:
    mret

    .data
msg:
    .string "Hello\n"

    .bss
exit_flag:
    .space 4
