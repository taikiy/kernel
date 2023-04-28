OS_IMG = os.bin
BOOT_IMG = boot.bin
KERNEL_IMG = kernel.bin

BUILD_DIR = ./build
SRC_DIR = ./src

ASRCS = $(shell find $(SRC_DIR) -name *.asm -not -path $(SRC_DIR)/boot/*)
CSRCS = $(shell find $(SRC_DIR) -name *.c)

# COBJS = $(CSRCS:%=$(BUILD_DIR)/%.o)
AOBJS = $(subst $(SRC_DIR),$(BUILD_DIR),$(ASRCS:.asm=.asm.o))
COBJS = $(subst $(SRC_DIR),$(BUILD_DIR),$(CSRCS:.c=.c.o))
OBJS = $(AOBJS) $(COBJS)

CC = i686-elf-gcc
ASM = nasm
LD = i686-elf-ld

INC_DIRS = $(shell find $(SRC_DIR) -type d -not -path $(SRC_DIR)/boot/*)
INCLUDES = $(addprefix -I,$(INC_DIRS))
AFLAGS = -f elf -g
CFLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parammeter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -std=gnu99
LDFLAGS = -g -relocatable

all: build mount

build: $(BUILD_DIR)/$(BOOT_IMG) $(BUILD_DIR)/$(KERNEL_IMG)
	rm -rf $(BUILD_DIR)/$(OS_IMG)
	dd if=$(BUILD_DIR)/$(BOOT_IMG) >> $(BUILD_DIR)/$(OS_IMG)
	dd if=$(BUILD_DIR)/$(KERNEL_IMG) >> $(BUILD_DIR)/$(OS_IMG)
	dd if=/dev/zero bs=1048576 count=16 >> $(BUILD_DIR)/$(OS_IMG)

$(BUILD_DIR)/$(BOOT_IMG): $(SRC_DIR)/boot/boot.asm
	mkdir -p $(dir $@)
	$(ASM) -f bin $(SRC_DIR)/boot/boot.asm -o $(BUILD_DIR)/$(BOOT_IMG)

$(BUILD_DIR)/$(KERNEL_IMG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/kernel.o
	$(CC) $(CFLAGS) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/$(KERNEL_IMG) $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	mkdir -p $(dir $@)
	$(ASM) $(AFLAGS) $< -o $@

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

run:
	qemu-system-x86_64 -hda $(BUILD_DIR)/$(OS_IMG)

mount:
	hdiutil attach -imagekey diskimage-class=CRawDiskImage -mount required $(BUILD_DIR)/$(OS_IMG)
	cp ./hello.txt "/Volumes/taiOS BOOT/"
	hdiutil detach `hdiutil info | tail -n 1 | cut -f 1`

clean:
	rm -rf $(BUILD_DIR)