#include <stdint.h>
#include <stdlib.h>

#include "decode.h"
#include "cpu.h"

DecodedInst decode(uint32_t inst)
{
    DecodedInst decoded;
    decoded.opcode = inst & 0x7F;         // [6:0]
    decoded.rd     = (inst >> 7) & 0x1F;  // [11:7]
    decoded.funct3 = (inst >> 12) & 0x7;  // [14:12]
    decoded.rs1    = (inst >> 15) & 0x1F; // [19:15]
    decoded.rs2    = (inst >> 20) & 0x1F; // [24:20]
    decoded.funct7 = (inst >> 25) & 0x7F; // [31:25]
    decoded.shamt  = (inst >> 20) & 0x1F; // [24:20]

    int32_t imm;
    switch(decoded.opcode) {
    case 0x23: // S-type: inst[31:25], inst[11:7]
        imm = ((inst >> 25) & 0x7F) << 5;
        imm |= (inst >> 7) & 0x1F;
        if(imm & 0x800)
            imm |= 0xFFFFF000;
        break;

    case 0x63: // B-type: inst[31], inst[7], inst[30:25], inst[11:8], bit0 = 0
        imm = ((inst >> 31) & 1) << 12;
        imm |= ((inst >> 7) & 1) << 11;
        imm |= ((inst >> 25) & 0x3F) << 5;
        imm |= ((inst >> 8) & 0xF) << 1;
        if(imm & 0x1000)
            imm |= 0xFFFFE000; // 负数符号扩展
        break;

    case 0x6F: // J-type: inst[31], inst[19:12], inst[20], inst[30:21], bit0 = 0
        imm = ((inst >> 31) & 1) << 20;
        imm |= ((inst >> 12) & 0xFF) << 12;
        imm |= ((inst >> 20) & 1) << 11;
        imm |= ((inst >> 21) & 0x3FF) << 1;
        if(imm & 0x100000)
            imm |= 0xFFE00000; // 负数符号扩展
        break;

    case 0x37:                              // LUI
    case 0x17:                              // AUIPC
        imm = (int32_t)(inst & 0xFFFFF000); // 提取高 20 位并符号扩展
        break;

    default:                         // I-type 和 R-type
        imm = (int32_t)(inst >> 20); // 提取立即数的高位
        if(imm & 0x800)
            imm |= 0xFFFFF000; // 负数符号扩展
        break;
    }

    decoded.imm = imm;

    return decoded;
}