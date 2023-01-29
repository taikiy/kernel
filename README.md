# Overview

Developing a multi-threaded kernel from scratch

# Sections

## Real Mode Development

### Printing a string to the screen

1. Edit `boot.asm`

2. Assemble

```
> nasm -f bin ./boot.asm -o ./boot.bin
```

`-f bin` assembled to binary. We don't use object file etc. because there's no concept of executables, file formats, etc. in the BIOS. Output has no header information. Just raw code output.

```
❯ ll
total 32
-rw-r--r--  1 taiki  staff   501B Jan 29 20:38 boot.asm
-rw-r--r--  1 taiki  staff   512B Jan 29 20:38 boot.bin
```

3. You can disassemble the bin file to see the contents

```
> ndisasm ./boot.bin`
00000000  B40E              mov ah,0xe
00000002  B041              mov al,0x41
00000004  CD10              int 0x10
00000006  EBFE              jmp short 0x6
00000008  0000              add [bx+si],al
...
000001FC  0000              add [bx+si],al
000001FE  55                push bp
000001FF  AA                stosb
```

4. Run

```
> qemu-system-x86_64 -hda ./boot.bin
```

[Print "A"](./img/real_mode/print_a.png?raw=true "Print A")
