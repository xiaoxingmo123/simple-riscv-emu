import struct

insts = [
    0x0FF00093,   # addi x1, x0, 255
    0x00102023,   # sw   x1, 0(x0)
    0x00002103,   # lw   x2, 0(x0)
    0x00002183,   # lb   x3, 0(x0)
    0x00000483,   # lbu  x4, 0(x0)   ← 修正：原来是 0x00002403 (x8)
    
    0xFFF00093,   # addi x1, x0, -1
    0x00101223,   # sh   x1, 4(x0)
    0x00401103,   # lh   x2, 4(x0)
    0x00405183,   # lhu  x3, 4(x0)
]

with open("test_mem.bin", "wb") as f:
    for i in insts:
        f.write(struct.pack("<I", i))