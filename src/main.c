#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define MEM_SIZE (1024 * 1024) // 1MB 内存

typedef union{
    uint8_t bytes[2];
    uint16_t u16;
}U16_t;

typedef union{
    uint8_t bytes[4];
    uint32_t u32;
}U32_t;

typedef struct{
    uint32_t regs[32];   /* x0 - x31 */
    uint32_t pc;         /* 程序计数器 */
}CPU;

/* 译码结果 */
typedef struct{
    uint32_t opcode;     /* [6:0] */
    uint32_t rd;         /* [11:7] */
    uint32_t funct3;     /* [14:12] */
    uint32_t rs1;        /* [19:15] */
    uint32_t rs2;        /* [24:20] */
    uint32_t funct7;     /* [31:25] */
    int32_t  imm;        /* 符号扩展后的立即数（先实现 I-type） */
    uint32_t shamt;      /* 新增：[24:20]，SLLI/SRLI/SRAI 的移位量 */
}DecodedInst;

uint8_t *memory;
CPU cpu;

/* 内存初始化，size 为字节数 */
uint8_t* mem_init(size_t size);

/* 从文件加载二进制到内存，返回加载字节数 */
size_t mem_load(const char *filename);

/* 读 8/16/32 位，小端序 */
uint8_t mem_read8(uint32_t addr);
uint16_t mem_read16(uint32_t addr);
uint32_t mem_read32(uint32_t addr);

/* 写 8/16/32 位，小端序 */
void mem_write8(uint32_t addr, uint8_t value);
void mem_write16(uint32_t addr, uint16_t value);
void mem_write32(uint32_t addr, uint32_t value);

/* 内存清理 */
void mem_free(void);

void cpu_init(CPU *cpu);
int cpu_execute(CPU *cpu, DecodedInst *inst);
/* 从当前 PC 取 32 位指令 */
uint32_t cpu_fetch(const CPU *cpu);
DecodedInst cpu_decode(uint32_t inst);

uint8_t* mem_init(size_t size)
{
    void* res = malloc(size);
    if(res == NULL)
    {
        fprintf(stderr, "内存不足, 分配内存失败\n");
    }
    return (uint8_t*)res;
}

size_t mem_load(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "cannot open file: %s\n", filename);
        return 0;
    }
    size_t bytes_read = fread(memory, 1, MEM_SIZE, file);
    fclose(file);
    return bytes_read;
}

uint8_t mem_read8(uint32_t addr)
{
    if(addr >= MEM_SIZE)
    {
        fprintf(stderr, "failed to read memory for out of index\n");
        return 0;
    }
    return memory[addr];
}

uint16_t mem_read16(uint32_t addr)
{
    U16_t data = {0};
    if(addr + 1 >= MEM_SIZE)
    {
        fprintf(stderr, "failed to read memory for out of index\n");
        return data.u16;
    }
    data.bytes[0] = memory[addr];     // 低字节
    data.bytes[1] = memory[addr + 1]; // 高字节
    return data.u16;
}

uint32_t mem_read32(uint32_t addr)
{
    U32_t data = {0};
    if(addr + 3 >= MEM_SIZE)
    {
        fprintf(stderr, "failed to read memory for out of index\n");
        return data.u32;
    }
    data.bytes[0] = memory[addr];
    data.bytes[1] = memory[addr + 1];
    data.bytes[2] = memory[addr + 2];
    data.bytes[3] = memory[addr + 3];
    return data.u32;
}

void mem_write8(uint32_t addr, uint8_t value)
{
    if(addr >= MEM_SIZE)
    {
        fprintf(stderr, "failed to write memory for out of index\n");
        return;
    }
    memory[addr] = value;
}

void mem_write16(uint32_t addr, uint16_t value)
{
    U16_t data;
    if(addr + 1 >= MEM_SIZE)
    {
        fprintf(stderr, "failed to write memory for out of index\n");
        return;
    }
    data.u16 = value;
    memory[addr] = data.bytes[0];
    memory[addr + 1] = data.bytes[1];
}

void mem_write32(uint32_t addr, uint32_t value)
{
    U32_t data;
    if(addr + 3 >= MEM_SIZE)
    {
        fprintf(stderr, "failed to write memory for out of index\n");
        return;
    }
    data.u32 = value;
    memory[addr] = data.bytes[0];
    memory[addr + 1] = data.bytes[1];
    memory[addr + 2] = data.bytes[2];
    memory[addr + 3] = data.bytes[3];
}

void mem_free(void)
{
    free(memory);
}

void cpu_init(CPU *cpu)
{
    memset(cpu->regs, 0, sizeof(cpu->regs)); // 初始化寄存器为 0
    cpu->pc = 0;                             // 程序计数器初始值为 0
}

uint32_t cpu_fetch(const CPU *cpu)
{
    return mem_read32(cpu -> pc);
}

DecodedInst cpu_decode(uint32_t inst)
{
    DecodedInst decoded;
    decoded.opcode = inst & 0x7F;           // [6:0]
    decoded.rd = (inst >> 7) & 0x1F;        // [11:7]
    decoded.funct3 = (inst >> 12) & 0x7;    // [14:12]
    decoded.rs1 = (inst >> 15) & 0x1F;      // [19:15]
    decoded.rs2 = (inst >> 20) & 0x1F;      // [24:20]
    decoded.funct7 = (inst >> 25) & 0x7F;   // [31:25]
    decoded.shamt = (inst >> 20) & 0x1F;    // [24:20]
    
    int32_t imm12;
    if(decoded.opcode == 0x23)
    {
        imm12 = ((inst >> 25) & 0x7F) << 5;
        imm12 |= (inst >> 7) & 0x1F;
        if (imm12 & 0x800) imm12 |= 0xFFFFF000;
    }
    else
    {
        imm12 = (int32_t)(inst >> 20); // 提取立即数的高位
        if (imm12 & 0x800) imm12 |= 0xFFFFF000; // 负数符号扩展
    }
    decoded.imm = imm12;

    return decoded;
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
        default:
            return -1;
    }
}

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
    
    size_t loaded_bytes = mem_load(argv[1]);
    cpu_init(&cpu);

    while (cpu.pc + 4 <= loaded_bytes) {
        uint32_t raw = cpu_fetch(&cpu);
        DecodedInst dcd = cpu_decode(raw);
        
        printf("PC=%#x: opcode=%#x, rd=%d, rs1=%d, rs2=%d, funct3=%#x, funct7=%#x, imm=%d\n",
            cpu.pc, dcd.opcode, dcd.rd, dcd.rs1, dcd.rs2, dcd.funct3, dcd.funct7, dcd.imm);
        
        if (cpu_execute(&cpu, &dcd) != 0) {
            printf("illegl cmd at PC=%#x, raw=%#x\n", cpu.pc, raw);
            printf("cmd: %#x\n", dcd.opcode);
            break;
        }
        cpu.pc += 4;
    }
    /* 执行结束后打印寄存器 */
    for (int i = 0; i < 32; i++) {
        printf("x%d = %u\n", i, cpu.regs[i]);
    }
    return 0;
}