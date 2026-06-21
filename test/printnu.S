.globl _start

.equ MEM_SIZE, 1 * 1024 * 1024
.equ UART_BASE, 0x10000000
.text

_start:
    li a0, 1234
    li a1, UART_BASE
    li sp, MEM_SIZE
    addi sp, sp, -100      # 开辟100B栈空间

    mv s0, sp
    mv t0, a0
    li t1, 0               # t1 是商

_div_loop:
    li t2, 10
    blt t0, t2, _push_digit    # 数字小于10直接压栈
    
    _sub_loop:
        blt t0, t2, _sub_done
        addi t0, t0, -10
        addi t1, t1, 1
        j _sub_loop
    
    _sub_done:
        addi t0, t0, '0'
        sb t0, 0(sp)
        addi sp, sp, 1
        mv t0, t1
        li t1, 0
        j _div_loop
   
    _push_digit:
        addi t0, t0, '0'
        sb t0, 0(sp)
        addi sp, sp, 1
    
_send_loop:
    addi sp, sp, -1
    blt sp, s0, _done
    lbu t3, 0(sp)
    sb t3, 0(a1)      # 发送t3
    j _send_loop

_done:
    addi t4, zero, '\n'
    sb t4, 0(a1)
    ebreak

