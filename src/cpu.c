#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "decode.h"
#include "memory.h"

void cpu_init(CPU *cpu, uint32_t entry) {
    memset(cpu->regs, 0, sizeof(cpu->regs)); // 初始化寄存器为 0
    cpu->pc = entry;                             // 程序计数器初始值为 0
}

uint32_t cpu_fetch(const CPU *cpu) { return mem_read32(cpu->pc); }

#define RD (cpu->regs[(inst->rd)])
#define WRITE_RD(val) (RD = inst->rd ? (val) : 0)
#define RS1 (cpu->regs[(inst->rs1)])
#define RS2 (cpu->regs[(inst->rs2)])
#define IMM (inst->imm)
#define SHAMT (inst->shamt)

static uint32_t read_csr(CPU *cpu, uint32_t addr);
static void write_csr(CPU *cpu, uint32_t addr, uint32_t val);

static ExecResult exec_rtype(CPU *cpu, DecodedInst *inst);
static ExecResult exec_itype_alu(CPU *cpu, DecodedInst *inst);
static ExecResult exec_load(CPU *cpu, DecodedInst *inst);
static ExecResult exec_store(CPU *cpu, DecodedInst *inst);
static ExecResult exec_btype(CPU *cpu, DecodedInst *inst);
static ExecResult exec_system(CPU *cpu, DecodedInst *inst);

ExecResult cpu_execute(CPU *cpu, DecodedInst *inst) {
    if(cpu == NULL || inst == NULL) {
        return EXEC_ILLEGAL_INST;
    }
    switch(inst->opcode) {
    /*R-type*/
    case 0x33:
        return exec_rtype(cpu, inst);
    /*I-type*/
    case 0x13:
        return exec_itype_alu(cpu, inst);
    /*Load*/
    case 0x03:
        return exec_load(cpu, inst);
    /*Store*/
    case 0x23:
        return exec_store(cpu, inst);
    /*LUI*/
    case 0x37:
        WRITE_RD((uint32_t)IMM);
        return 0;
    /*AUIPC*/
    case 0x17:
        WRITE_RD(cpu->pc + (uint32_t)IMM);
        return 0;
    /*JAL*/
    case 0x6F:
        WRITE_RD(cpu->pc + 4);
        cpu->pc += (uint32_t)IMM;
        return 0;
    /*JARL*/
    case 0x67:
        if(inst->funct3 == 0x00) {
            WRITE_RD(cpu->pc + 4);
            cpu->pc = (RS1 + (uint32_t)IMM) & ~1u;
            return 0;
        }
        return -1;
    /*BRANCH*/
    case 0x63:
        return exec_btype(cpu, inst);
    /*SYSTEM*/
    case 0x73:
        return exec_system(cpu, inst);
    default:
        return EXEC_ILLEGAL_INST;
    }
}

void cpu_trap(CPU *cpu, uint32_t cause, uint32_t tval) {
    cpu->mcause = cause;
    cpu->mtval  = tval;
    cpu->mepc   = cpu->pc;
    cpu->pc     = cpu->mtvec;
}

static uint32_t read_csr(CPU *cpu, uint32_t addr) {
    switch(addr) {
    case CSR_MSTATUS:
        return cpu->mstatus;
    case CSR_MEPC:
        return cpu->mepc;
    case CSR_MCAUSE:
        return cpu->mcause;
    case CSR_MTVAL:
        return cpu->mtval;
    case CSR_MTVEC:
        return cpu->mtvec;
    default:
        return 0;
    }
}

static void write_csr(CPU *cpu, uint32_t addr, uint32_t val) {
    switch(addr) {
    case CSR_MSTATUS:
        cpu->mstatus = val;
        break;
    case CSR_MEPC:
        cpu->mepc = val;
        break;
    case CSR_MCAUSE:
        cpu->mcause = val;
        break;
    case CSR_MTVAL:
        cpu->mtval = val;
        break;
    case CSR_MTVEC:
        cpu->mtvec = val;
        break;
    }
}

static ExecResult exec_rtype(CPU *cpu, DecodedInst *inst) {
    switch(inst->funct3) {
    case 0x00:
        if(inst->funct7 == 0x00) {
            WRITE_RD(RS1 + RS2);
            return EXEC_SUCCESS;
        } else if(inst->funct7 == 0x20) {
            WRITE_RD(RS1 - RS2);
            return EXEC_SUCCESS;
        } else {
            return EXEC_ILLEGAL_INST;
        }
    case 0x01:
        WRITE_RD(RS1 << (RS2 & 0x1F));
        return EXEC_SUCCESS;
    case 0x02:
        WRITE_RD((int32_t)RS1 < (int32_t)RS2 ? 1u : 0u);
        return EXEC_SUCCESS;
    case 0x03:
        WRITE_RD(RS1 < RS2 ? 1u : 0u);
        return EXEC_SUCCESS;
    case 0x04:
        WRITE_RD(RS1 ^ RS2);
        return EXEC_SUCCESS;
    case 0x05:
        if(inst->funct7 == 0x00) {
            WRITE_RD(RS1 >> (RS2 & 0x1F));
            return EXEC_SUCCESS;
        } else if(inst->funct7 == 0x20) {
            WRITE_RD((int32_t)RS1 >> (RS2 & 0x1F));
            return EXEC_SUCCESS;
        }
        return EXEC_ILLEGAL_INST;
    case 0x06:
        WRITE_RD(RS1 | RS2);
        return EXEC_SUCCESS;
    case 0x07:
        WRITE_RD(RS1 & RS2);
        return EXEC_SUCCESS;
    default:
        return EXEC_ILLEGAL_INST;
    }
    return EXEC_ILLEGAL_INST; // Should never reach here
}

static ExecResult exec_itype_alu(CPU *cpu, DecodedInst *inst) {
    switch(inst->funct3) {
    case 0x00:
        WRITE_RD(RS1 + IMM);
        return EXEC_SUCCESS;
    case 0x02:
        WRITE_RD((int32_t)RS1 < (int32_t)IMM ? 1u : 0u);
        return EXEC_SUCCESS;
    case 0x03:
        WRITE_RD(RS1 < (uint32_t)IMM ? 1u : 0u);
        return EXEC_SUCCESS;
    case 0x04:
        WRITE_RD(RS1 ^ IMM);
        return EXEC_SUCCESS;
    case 0x06:
        WRITE_RD(RS1 | IMM);
        return EXEC_SUCCESS;
    case 0x07:
        WRITE_RD(RS1 & IMM);
        return EXEC_SUCCESS;
    case 0x01:
        if(inst->funct7 != 0x00)
            return EXEC_ILLEGAL_INST;
        WRITE_RD(RS1 << SHAMT);
        return EXEC_SUCCESS;
    case 0x05:
        if(inst->funct7 == 0x00) {
            WRITE_RD(RS1 >> SHAMT);
            return EXEC_SUCCESS;
        } else if(inst->funct7 == 0x20) {
            WRITE_RD((int32_t)RS1 >> SHAMT);
            return EXEC_SUCCESS;
        }
        return EXEC_ILLEGAL_INST;
    default:
        return EXEC_ILLEGAL_INST;
    }
}

static ExecResult exec_load(CPU *cpu, DecodedInst *inst) {
    switch(inst->funct3) {
    case 0x00:
        WRITE_RD((int8_t)mem_read8(RS1 + IMM));
        return EXEC_SUCCESS;
    case 0x01:
        WRITE_RD((int16_t)mem_read16(RS1 + IMM));
        return EXEC_SUCCESS;
    case 0x02:
        WRITE_RD(mem_read32(RS1 + IMM));
        return EXEC_SUCCESS;
    case 0x04:
        WRITE_RD(mem_read8(RS1 + IMM));
        return EXEC_SUCCESS;
    case 0x05:
        WRITE_RD(mem_read16(RS1 + IMM));
        return EXEC_SUCCESS;
    default:
        return EXEC_ILLEGAL_INST;
    }
}

static ExecResult exec_store(CPU *cpu, DecodedInst *inst) {
    switch(inst->funct3) {
    case 0x00:
        mem_write8(RS1 + IMM, (uint8_t)(RS2 & 0xFF));
        return EXEC_SUCCESS;
    case 0x01:
        mem_write16(RS1 + IMM, (uint16_t)(RS2 & 0xFFFF));
        return EXEC_SUCCESS;
    case 0x02:
        mem_write32(RS1 + IMM, RS2);
        return EXEC_SUCCESS;
    default:
        return EXEC_ILLEGAL_INST;
    }
}

static ExecResult exec_btype(CPU *cpu, DecodedInst *inst) {
    switch(inst->funct3) {
    case 0x00:
        if(RS1 == RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;
    case 0x01:
        if(RS1 != RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;
    case 0x04:
        if((int32_t)RS1 < (int32_t)RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;
    case 0x05:
        if((int32_t)RS1 >= (int32_t)RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;
    case 0x06:
        if(RS1 < RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;
    case 0x07:
        if(RS1 >= RS2)
            cpu->pc += (uint32_t)IMM;
        return EXEC_SUCCESS;

    default:
        return EXEC_ILLEGAL_INST;
    }
}

static ExecResult exec_system(CPU *cpu, DecodedInst *inst) {
    if(inst->funct3 == 0) {
        // ECALL / EBREAK / MRET
        uint32_t funct12 = (inst->imm & 0xFFF);
        if(funct12 == 0x000) {
            // ECALL
            cpu_trap(cpu, MCAUSE_ECALL_M, 0);
            return EXEC_SUCCESS;
        } else if(funct12 == 0x001) {
            // EBREAK
            return EXEC_EBREAK_HALT; // 返回 EXEC_EBREAK_HALT 表示遇到 EBREAK，正常停机
        } else if(funct12 == 0x302) {
            // MRET
            cpu->pc = cpu->mepc;
            return EXEC_SUCCESS;
        } else {
            return EXEC_ILLEGAL_INST; // 未知的 SYSTEM 指令
        }
    }

    // CSR 指令
    if(inst->rd != 0) {
        switch(inst->imm & 0xFFF) {
        case CSR_MSTATUS:
            WRITE_RD(cpu->mstatus);
            break;
        case CSR_MEPC:
            WRITE_RD(cpu->mepc);
            break;
        case CSR_MCAUSE:
            WRITE_RD(cpu->mcause);
            break;
        case CSR_MTVAL:
            WRITE_RD(cpu->mtval);
            break;
        case CSR_MTVEC:
            WRITE_RD(cpu->mtvec);
            break;
        default:
            return EXEC_ILLEGAL_INST; // 未知的 CSR 地址
        }
    }
    uint32_t csr_addr = inst->imm & 0xFFF;
    uint32_t write_val;
    if(inst->funct3 & 0x4) {
        write_val = inst->rs1; // 这里rs1是立即数而非寄存器索引
    } else {
        write_val = RS1;
    }
    switch(inst->funct3) {
    case 0x1: // CSRRW
    case 0x5: // CSRRWI
        write_csr(cpu, csr_addr, write_val);
        break;
    case 0x2: // CSRRS
    case 0x6: // CSRRSI
        write_csr(cpu, csr_addr, read_csr(cpu, csr_addr) | write_val);
        break;
    case 0x3: // CSRRC
    case 0x7: // CSRRCI
        if(write_val != 0) {
            write_csr(cpu, csr_addr, read_csr(cpu, csr_addr) & ~write_val);
        }
        break;
    default:
        return EXEC_ILLEGAL_INST; // 未知的 CSR 指令类型
    }
    return EXEC_SUCCESS;
}