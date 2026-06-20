#include <stdio.h>
#include <stdlib.h>

#include "../inc/elf.h"
#include "../inc/memory.h"

uint32_t elf_load(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror("fopen"); exit(1); }

    /* 1. 读 ELF Header */
    Elf32_Ehdr hdr;
    fread(&hdr, sizeof(hdr), 1, fp);

    /* 验证魔数：0x7F 'E' 'L' 'F' */
    if (hdr.e_ident[0] != 0x7F || hdr.e_ident[1] != 'E' ||
        hdr.e_ident[2] != 'L'  || hdr.e_ident[3] != 'F') {
        fprintf(stderr, "Not an ELF file\n");
        exit(1);
    }

    /* 验证 32 位、小端、RISC-V */
    if (hdr.e_ident[4] != 1) { /* 32-bit */ }
    if (hdr.e_ident[5] != 1) { /* little-endian */ }
    if (hdr.e_machine != 0xF3) { /* RISC-V */ }

    /* 2. 遍历 Program Header，加载 PT_LOAD 段 */
    fseek(fp, hdr.e_phoff, SEEK_SET);
    for (int i = 0; i < hdr.e_phnum; i++) {
        Elf32_Phdr ph;
        fread(&ph, sizeof(ph), 1, fp);

        if (ph.p_type != 1) continue;  /* PT_LOAD = 1 */

        /* 文件数据 → 内存 */
        fseek(fp, ph.p_offset, SEEK_SET);
        for (uint32_t j = 0; j < ph.p_filesz; j++) {
            uint8_t byte;
            fread(&byte, 1, 1, fp);
            mem_write8(ph.p_vaddr + j, byte);
        }

        /* 内存大小 > 文件大小，补零（.bss 等）*/
        for (uint32_t j = ph.p_filesz; j < ph.p_memsz; j++) {
            mem_write8(ph.p_vaddr + j, 0);
        }
    }

    fclose(fp);
    return hdr.e_entry;  /* 返回入口地址 */
}