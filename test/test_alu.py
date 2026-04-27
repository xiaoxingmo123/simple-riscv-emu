import struct

# 指令编码（小端序写入文件）
insts = [
    0x02a00093,   # addi x1, x0, 42
    0x06400113,   # addi x2, x0, 100
    0x002081b3,   # add  x3, x1, x2
    0x40118233,   # sub  x4, x3, x1   (x4 = 142-42 = 100)
    0x00f0f293,   # andi x5, x1, 15   (x5 = 42 & 15 = 10)
    0x002092b3,   # slli x5, x1, 2    (x5 = 42 << 2 = 168)  ← 覆盖 x5
]
with open("test_alu.bin", "wb") as f:
    for i in insts:
        f.write(struct.pack("<I", i))