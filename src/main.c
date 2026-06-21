#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "main.h"
#include "elf.h"
#include "cpu.h"
#include "memory.h"
#include "decode.h"


uint8_t *memory;
CPU cpu;

// 暂时规定第二个参数是文件路径
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        return -1;
    }
    memory = mem_init(MEM_SIZE); // 初始化 1MB 内存
    if (memory == NULL)
    {
        return -1; // 内存分配失败，退出程序
    }
    
    cpu_init(&cpu);

    while (1) {
        uint32_t raw = cpu_fetch(&cpu);
        DecodedInst dcd = decode(raw);
        uint32_t current_pc = cpu.pc;
        
        // printf("PC=%#x: opcode=%#x, rd=%d, rs1=%d, rs2=%d, funct3=%#x, funct7=%#x, imm=%d\n",
        //     cpu.pc, dcd.opcode, dcd.rd, dcd.rs1, dcd.rs2, dcd.funct3, dcd.funct7, dcd.imm);
        
        int ret = cpu_execute(&cpu, &dcd);
        if (ret == 1) {  /* EBREAK 正常停机 */
            printf("EBREAK at PC=%#x, halt.\n", current_pc);
            break;
        }
        if (ret != 0) {
            printf("illegal cmd at PC=%#x, raw=%#x\n", current_pc, raw);
            break;
        }
        
        /* execute 未修改 pc 时，顺序执行 */
        if (cpu.pc == current_pc) {
            cpu.pc += 4;
        }
    }
    /* 执行结束后打印寄存器 */
    for (int i = 0; i < 32; i++) {
        printf("x%d = %u\n", i, cpu.regs[i]);
    }
    return 0;
}