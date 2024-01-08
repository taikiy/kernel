TARGET = hello
OBJ = hello.o

BUILD_DIR = ./build
SRC_DIR = ./src

LINKER_FILE = $(SRC_DIR)/linker.ld

ASRCS = $(shell find $(SRC_DIR) -name *.asm)
CSRCS = $(shell find $(SRC_DIR) -name *.c)

AOBJS = $(subst $(SRC_DIR),$(BUILD_DIR),$(ASRCS:.asm=.asm.o))
COBJS = $(subst $(SRC_DIR),$(BUILD_DIR),$(CSRCS:.c=.c.o))
OBJS = $(AOBJS) $(COBJS)

LIBS = ../stdlib/build/stdlib.o
INCLUDES = -I../stdlib/src

CC = i686-elf-gcc
ASM = nasm
LD = i686-elf-ld

AFLAGS = -f elf -g
CFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parammeter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -std=gnu99
LDFLAGS = -relocatable

all: build

build: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/$(OBJ)
	$(CC) $(CFLAGS) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJ) $(LIBS)

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	mkdir -p $(dir $@)
	$(ASM) $(AFLAGS) $< -o $@

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
