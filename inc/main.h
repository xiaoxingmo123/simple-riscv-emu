#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <stddef.h>

#include "cpu.h"

#define UART_TX 0x10000000     // UART 发送寄存器地址
#define UART_RX 0x10000000     // UART 接收寄存器地址

typedef union{
    uint8_t bytes[2];
    uint16_t u16;
}U16_t;

typedef union{
    uint8_t bytes[4];
    uint32_t u32;
}U32_t;

extern uint8_t *memory;
extern CPU cpu;

#endif /* __MAIN_H__ */