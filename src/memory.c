#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "cpu.h"
#include "../inc/memory.h"

size_t mem_load(const char *filename, CPU *cpu)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "cannot open file: %s\n", filename);
        return 0;
    }
    if (strstr(filename, ".bin")) {
        /* 纯二进制：从地址 0 加载，入口 0 */
        size_t bytes_read = fread(memory, 1, MEM_SIZE, file);
        cpu -> pc = 0;
        fclose(file);
        return bytes_read;
    } else {
        /* ELF：解析加载，入口从 ELF 头读取 */
        uint32_t entry = elf_load(filename);
        cpu -> pc = entry;
    }
    
}

uint8_t mem_read8(uint32_t addr)
{
    if(addr == UART_RX)
    {
        return 0; //TODO: 暂时简化 
    }
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
    if(addr == UART_TX)
    {
        putchar(value);
        fflush(stdout);
        return;
    }
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

uint8_t* mem_init(size_t size)
{
    void* res = malloc(size);
    if(res == NULL)
    {
        fprintf(stderr, "内存不足, 分配内存失败\n");
    }
    return (uint8_t*)res;
}