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
} CPU;

typedef enum {
    EXEC_SUCCESS = 0,
    EXEC_EBREAK_HALT,
    EXEC_ILLEGAL_INST,
} ExecResult;

// 异常原因编码
#define MCAUSE_ECALL_M 11
#define MCAUSE_ECALL_S 9
#define MAUCSE_ECALL_U 8
#define MCAUSE_ILLEGAL_INST 2

// CRS 地址
#define CSR_MSTATUS 0x300
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MTVEC 0x305

void cpu_init(CPU *cpu, uint32_t entry);
uint32_t cpu_fetch(const CPU *cpu);
ExecResult cpu_execute(CPU *cpu, DecodedInst *inst);
// 触发异常
void cpu_trap(CPU *cpu, uint32_t cause, uint32_t tval);
#endif /* __CPU_H__ */