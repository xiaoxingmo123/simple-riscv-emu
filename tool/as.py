#!/usr/bin/python3
"""
riscv_assemble.py —— 把 RISC-V 汇编文件编译成纯二进制

用法: python3 riscv_assemble.py <input.s> [output.bin]
"""

import sys
import subprocess
import os
import tempfile
import shutil

# 工具链前缀，如果安装路径不同请修改
TOOLCHAIN_PREFIX = "riscv64-unknown-elf-"

def run_cmd(cmd, description):
    """运行命令，出错时打印信息并退出"""
    print(f"  → {description}: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"错误: {description} 失败")
        print("STDERR:", result.stderr)
        sys.exit(1)
    return result

def assemble(input_file, output_file=None):
    # 确定输出文件名
    if output_file is None:
        base = os.path.splitext(input_file)[0]
        output_file = base + ".bin"

    # 创建临时目录
    with tempfile.TemporaryDirectory() as tmpdir:
        obj_file = os.path.join(tmpdir, "tmp.o")
        elf_file = os.path.join(tmpdir, "tmp.elf")

        # 1. 汇编
        run_cmd([
            f"{TOOLCHAIN_PREFIX}as",
            "-march=rv32ima_zicsr", "-mabi=ilp32",
            "-g",
            input_file, "-o", obj_file
        ], "汇编")

        # 2. 链接
        run_cmd([
            f"{TOOLCHAIN_PREFIX}ld",
            "-melf32lriscv",
            "-T", "./link.ld",
            obj_file, "-o", elf_file
        ], "链接")

        # 3. 提取二进制
        run_cmd([
            f"{TOOLCHAIN_PREFIX}objcopy",
            "-O", "binary",
            elf_file, output_file
        ], "提取二进制")

        # 可选：显示反汇编
        result = subprocess.run([
            f"{TOOLCHAIN_PREFIX}objdump",
            "-d", "-M", "no-aliases",
            elf_file
        ], capture_output=True, text=True)
        if result.returncode == 0:
            print("\n反汇编:")
            print(result.stdout)

    # 显示输出信息
    size = os.path.getsize(output_file)
    print(f"\n完成: {output_file} ({size} 字节)")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python3 riscv_assemble.py <input.s> [output.bin]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(input_file):
        print(f"错误: 找不到文件 {input_file}")
        sys.exit(1)

    assemble(input_file, output_file)
