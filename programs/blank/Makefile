TARGET = blank.bin
OBJ = blank.o

BUILD_DIR = ./build
SRC_DIR = ./src

LINKER_FILE = $(SRC_DIR)/linker.ld

ASRCS = $(shell find $(SRC_DIR) -name *.asm)

AOBJS = $(subst $(SRC_DIR),$(BUILD_DIR),$(ASRCS:.asm=.asm.o))
OBJS = $(AOBJS)

CC = i686-elf-gcc
ASM = nasm
LD = i686-elf-ld

AFLAGS = -f elf -g
CFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parammeter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -std=gnu99
LDFLAGS = -g -relocatable

all: build

build: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/$(OBJ)
	$(CC) $(CFLAGS) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(OBJ)

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	mkdir -p $(dir $@)
	$(ASM) $(AFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)