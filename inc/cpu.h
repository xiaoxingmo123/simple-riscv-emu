#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stddef.h>

#include "decode.h"

typedef struct {
    uint32_t regs[32]; /* x0 - x31 */
    uint32_t pc;       /* 程序计数器 */
    uint32_t mstatus;  /* 机器状态寄存器 */
    uint32_t mepc;     /*异常PC*/
    uint32_t mcause;   /* 异常原因寄存器 */
    uint32_t mtval;    /* 异常值寄存器 */
    uint32_t mtvec;    /* 异常向量表基地址 */
    uint32_t mie;      /* 中断使能寄存器 */
    uint32_t mip;      /* 中断挂起寄存器 */
} CPU;

typedef enum {
    EXEC_SUCCESS = 0,
    EXEC_EBREAK_HALT,
    EXEC_ILLEGAL_INST,
} ExecResult;

// 异常原因编码
#define MCAUSE_ECALL_M 11
#define MCAUSE_ECALL_S 9
#define MCAUSE_ECALL_U 8
#define MCAUSE_ILLEGAL_INST 2

// CRS 地址
#define CSR_MSTATUS 0x300  // （Machine Status），维护各种状态，如全局中断使能
#define CSR_MEPC 0x341     // （Machine Exception PC），指向发生异常的指令
#define CSR_MCAUSE 0x342   // （Machine Exception Cause），指示发生了何种异常
#define CSR_MTVAL 0x343    // （Machine Trap Value），存放当前自陷相关的额外信息
#define CSR_MTVEC 0x305    // （Machine Trap Vector），存放发生异常时处理器跳转的地址
#define CSR_MIE 0x304      // （Machine Interrupt Enable），维护处理器的中断使能状态
#define CSR_MIP 0x344      // （Machine Interrupt Pending）， 记录当前的中断请求
#define CSR_MSCRATCH 0x340 // （Machine Scratch），用于存放临时数据，通常用于异常处理

#define MSTATUS_MIE (0x1 << 3)
#define MSTATUS_MPIE (0x1 << 7) // machine previous interrupt enable

// 特殊寄存器地址
#define UART_TX 0x10000000      // UART 发送寄存器地址
#define UART_RX 0x10000000      // UART 接收寄存器地址
#define MTIME_ADDR 0x200BFF8    // 定时器寄存器地址
#define MTIMECMP_ADDR 0x2004000 // 定时器比较寄存器地址

void cpu_init(CPU *cpu, uint32_t entry);
uint32_t cpu_fetch(const CPU *cpu);
ExecResult cpu_execute(CPU *cpu, DecodedInst *inst);
// 触发异常
void cpu_trap(CPU *cpu, uint32_t cause, uint32_t tval, int is_interrupt);
#endif /* __CPU_H__ */