# 适用与.h与.c处于同一文件的工程
CROSS = arm-none-linux-gnueabi-
CC    := $(CROSS)gcc
AR    := $(CROSS)ar
LD    := $(CROSS)ld
STRIP := $(CROSS)strip


SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(notdir ${SRC}))


TARGET = main
CFLAGS = -g -Wall
# 调试阶段加-g参数，确定后去掉该参数
${TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm *.o ${TARGET}