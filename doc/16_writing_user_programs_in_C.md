# Writing User Programs in C

## Overview

Enabling users to write their own programs in C requires _stdlib_ which provides a set of functions for memory allocation, string manipulation, and other functions which involves calling the kernel system calls. The _stdlib_ library is not part of the kernel, so we need to implement it. Any user program written in C must be linked with the _stdlib_ library.

The very first step in implementing the _stdlib_ is the entry point of a user program, which is the _main_ function. In the assembly code, we declare `_start` as the entry point of a user program. The `_start` function calls `main` which is declared as `extern` in the assembly code. The `main` function is defined in the user's C code.

The user program file is processed by our ELF loader, where the entry point is set to _stdlib_'s `_start` address. The `_start` function then calls `main` which is the entry point of the user's C code.

- Our first stdlib [commit](https://github.com/taikiy/kernel/commit/22984b790428a6732a0037bdb52c9d939514b314)

Note that running `hello` in this commit will cause a page fault. This is because we have not implemented the `exit` system call yet. The `exit` system call is called when the `main` function returns. The `exit` system call will be responsible for terminating tasks and the process and freeing the memory allocated by the user program.

## print function

The first function we implement is the `print` function. The `print` function is a wrapper of the INT80h command 1, `sys_print`, system call.

- stdlib `print` function [commit](https://github.com/taikiy/kernel/commit/47a2fef03b73645c393992245d160666e08a44cd)
- `getchar`, `putchar` [commit]()
