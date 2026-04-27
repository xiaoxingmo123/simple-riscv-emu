CC       := gcc
CFLAGS   := -Wall -Wextra
LDFLAGS  := 

SRC_DIR  := src
INC_DIR  := inc
OBJ_DIR  := obj

TARGET   := riscv_emu

SOURCES  := $(wildcard $(SRC_DIR)/*.c)
OBJECTS  := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
DEPENDS := $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "compile succeed: $@"

# ---------- 编译：将 src/%.c 编译成 obj/%.o ----------
# 注意：使用了 -MMD -MP 自动生成依赖文件（.d），并保存在 obj/ 目录下
# -I$(INC_DIR) 告诉编译器去哪里找头文件
# -c 表示只编译不链接
# -o $@ 指定输出文件（.o 文件）
# $< 表示第一个依赖（即 .c 文件）
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "clean succeed"

-include $(DEPENDS)

.PHONY: all clean