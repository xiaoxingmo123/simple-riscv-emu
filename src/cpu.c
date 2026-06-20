#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "memory.h"
#include "decode.h"

void cpu_init(CPU *cpu)
{
    memset(cpu->regs, 0, sizeof(cpu->regs)); // 初始化寄存器为 0
    cpu->pc = 0;                             // 程序计数器初始值为 0
}

uint32_t cpu_fetch(const CPU *cpu)
{
    return mem_read32(cpu -> pc);
}

#define RD (cpu -> regs[(inst -> rd)])
#define WRITE_RD(val) (RD = inst -> rd ? (val): 0)
#define RS1 (cpu -> regs[(inst -> rs1)])
#define RS2 (cpu -> regs[(inst -> rs2)])
#define IMM (inst -> imm)
#define SHAMT (inst -> shamt)

int cpu_execute(CPU *cpu, DecodedInst *inst)
{
    if(cpu == NULL || inst == NULL)
    {
        return -1;
    }
    switch(inst -> opcode)
    {
        /*R-type*/
        case 0x33:
            switch(inst -> funct3)
            {
                case 0x00:
                    if(inst -> funct7 == 0x00)
                    {
                        WRITE_RD(RS1 + RS2);
                        return 0;
                    }
                    else if(inst -> funct7 == 0x20)
                    {
                        WRITE_RD(RS1 - RS2);
                        return 0;
                    }
                    return -1;
                case 0x01: WRITE_RD(RS1 << (RS2 & 0x1F));                   return 0;
                case 0x02: WRITE_RD((int32_t)RS1 < (int32_t)RS2 ? 1u : 0u); return 0;
                case 0x03: WRITE_RD(RS1 < RS2 ? 1u: 0u);                    return 0;
                case 0x04: WRITE_RD(RS1 ^ RS2);                             return 0;
                case 0x05:
                    if(inst -> funct7 == 0x00)
                    {
                        WRITE_RD(RS1 >> (RS2 & 0x1F));
                        return 0;
                    }
                    else if(inst -> funct7 == 0x20)
                    {
                        WRITE_RD((int32_t)RS1 >> (RS2 & 0x1F));
                        return 0;
                    }
                    return -1;
                case 0x06: WRITE_RD(RS1 | RS2);                             return 0;
                case 0x07: WRITE_RD(RS1 & RS2);                             return 0;
                default:
                    return -1;
            }
            break;
        /*I-type*/
        case 0x13:
            switch(inst -> funct3)
            {
                case 0x00: WRITE_RD(RS1 + IMM);                             return 0;
                case 0x02: WRITE_RD((int32_t)RS1 < (int32_t)IMM ? 1u : 0u); return 0;
                case 0x03: WRITE_RD(RS1 < (uint32_t)IMM ? 1u: 0u);          return 0;
                case 0x04: WRITE_RD(RS1 ^ IMM);                             return 0;
                case 0x06: WRITE_RD(RS1 | IMM);                             return 0;
                case 0x07: WRITE_RD(RS1 & IMM);                             return 0;
                case 0x01: 
                    if(inst->funct7 != 0x00) return -1;
                    WRITE_RD(RS1 << SHAMT);
                    return 0;
                case 0x05:
                    if(inst -> funct7 == 0x00)
                    {
                        WRITE_RD(RS1 >> SHAMT);
                        return 0;
                    }
                    else if(inst -> funct7 == 0x20)
                    {
                        WRITE_RD((int32_t)RS1 >> SHAMT);
                        return 0; 
                    }
                    return -1;
                default:
                    return -1;
            }
            break;
        /*Load*/
        case 0x03:
            switch(inst -> funct3)
            {
                case 0x00: WRITE_RD((int8_t)mem_read8(RS1 + IMM));           return 0;
                case 0x01: WRITE_RD((int16_t)mem_read16(RS1 + IMM));         return 0;
                case 0x02: WRITE_RD(mem_read32(RS1 + IMM));                  return 0;
                case 0x04: WRITE_RD(mem_read8(RS1 + IMM));                   return 0;
                case 0x05: WRITE_RD(mem_read16(RS1 + IMM));                  return 0;

                default:                                                     return -1;
            }
            break;
        /*Store*/
        case 0x23:
            switch(inst -> funct3)
            {
                case 0x00: mem_write8(RS1 + IMM, (uint8_t)(RS2 & 0xFF));     return 0;
                case 0x01: mem_write16(RS1 + IMM, (uint16_t)(RS2 & 0xFFFF)); return 0;
                case 0x02: mem_write32(RS1 + IMM, RS2);                      return 0;

                default:                                                     return -1;
            }

        /*LUI*/
        case 0x37: WRITE_RD((uint32_t)IMM);                                  return 0;
        
        /*AUIPC*/
        case 0x17: WRITE_RD(cpu -> pc + (uint32_t)IMM);                      return 0;

        /*JAL*/
        case 0x6F: WRITE_RD(cpu -> pc + 4); cpu -> pc += (uint32_t)IMM;      return 0;
        
        /*JARL*/
        case 0x67:
            if(inst -> funct3 == 0x00)
            {
                WRITE_RD(cpu -> pc + 4);
                cpu -> pc = (RS1 + (uint32_t)IMM) & ~1u;
                return 0;
            }
            return -1;
        
            /*BRANCH*/
            case 0x63:
                switch(inst -> funct3)
                {
                    case 0x00: if(RS1 == RS2) cpu -> pc += (uint32_t)IMM;                   return 0;
                    case 0x01: if(RS1 != RS2) cpu -> pc += (uint32_t)IMM;                   return 0;
                    case 0x04: if((int32_t)RS1 < (int32_t)RS2) cpu -> pc += (uint32_t)IMM;  return 0;
                    case 0x05: if((int32_t)RS1 >= (int32_t)RS2) cpu -> pc += (uint32_t)IMM; return 0;
                    case 0x06: if(RS1 < RS2) cpu -> pc += (uint32_t)IMM;                    return 0;
                    case 0x07: if(RS1 >= RS2) cpu -> pc += (uint32_t)IMM;                   return 0;

                    default:                                                                return -1;
                }

            /*SYSTEM*/
            case 0x73:
                // EBREAK
                if(inst -> funct3 == 0 && inst -> imm == 1) return 1;
                return -1;
        default:
            return -1;
    }
}