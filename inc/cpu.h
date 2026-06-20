#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stddef.h>

#include "decode.h"

typedef struct{
    uint32_t regs[32];   /* x0 - x31 */
    uint32_t pc;         /* 程序计数器 */
}CPU;

void cpu_init(CPU *cpu);
uint32_t cpu_fetch(const CPU *cpu);
int cpu_execute(CPU *cpu, DecodedInst *inst);

#endif /* __CPU_H__ */