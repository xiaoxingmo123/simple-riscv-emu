# gen_test.py - 生成测试用的 RISC-V 二进制
import struct

# addi x1, x0, 42   ->  0x02a00093
# addi x2, x0, 100  ->  0x06400113
# add x3, x1, x2    ->  0x002081b3
instructions = [0x02a00093, 0x06400113, 0x002081b3]

with open("test.bin", "wb") as f:
    for inst in instructions:
        f.write(struct.pack("<I", inst))  # 小端序写入