import struct

UART_BASE = 0x10000000

def lui(rd, imm):
    return (imm & 0xFFFFF000) | (rd << 7) | 0x37

def addi(rd, rs1, imm):
    imm = imm & 0xFFF
    return (imm << 20) | (rs1 << 15) | (0 << 12) | (rd << 7) | 0x13

def sb(rs2, rs1, imm):
    imm = imm & 0xFFF
    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (0 << 12) | ((imm & 0x1F) << 7) | 0x23

def ebreak():
    return 0x00100073

# a0 = x10, a1 = x11
insts = [
    lui(10, UART_BASE),           # lui  a0, 0x10000      (a0 = 0x10000000)
    addi(11, 0, ord('H')),        # addi a1, x0, 'H'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    addi(11, 0, ord('e')),        # addi a1, x0, 'e'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    addi(11, 0, ord('l')),        # addi a1, x0, 'l'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    addi(11, 0, ord('l')),        # addi a1, x0, 'l'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    addi(11, 0, ord('o')),        # addi a1, x0, 'o'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    addi(11, 0, ord('\n')),       # addi a1, x0, '\n'
    sb(11, 10, 0),                # sb   a1, 0(a0)
    ebreak(),                     # ebreak
]

with open("test_uart.bin", "wb") as f:
    for i in insts:
        f.write(struct.pack("<I", i))