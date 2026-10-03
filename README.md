# 简单的 Risc-V 模拟器，支持基础整数指令集和部分 CSR 寄存器

## 使用
```bash
make
./riscv_emu <BIN | ELF>
```

## 编译测试代码
```bash
python3 tool/as.py <test.s>
```