#ifndef __DECODE_H__
#define __DECODE_H__


#include <stdint.h>
#include <stddef.h>

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

DecodedInst decode(uint32_t inst);

#endif /* __DECODE_H__ */