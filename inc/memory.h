#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"
#include "main.h"

#define MEM_SIZE (1024 * 1024) // 1MB 内存

/* 内存初始化，size 为字节数 */
uint8_t* mem_init(size_t size);

/* 从文件加载二进制到内存 */
void mem_load(const char *filename, CPU *cpu);

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

#endif /* __MEMORY_H__ */