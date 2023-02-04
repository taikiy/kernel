all:
	nasm -f bin ./boot.asm -o ./boot.bin
	dd if=./message.txt >> ./boot.bin
	dd if=/dev/zero bs=$(shell echo $$((512 - `stat message.txt | cut -d ' ' -f 8`))) count=1 >> ./boot.bin

clean:
	rm -rf ./boot.bin