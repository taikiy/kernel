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
- `getchar`, `putchar` [commit](https://github.com/taikiy/kernel/commit/ac10715ca82860e0589c04585c4ed57303be113c)

## malloc & free functions

Implementing `malloc` and `free` functions is similar to what we did with the `print` function. Each time the `malloc` function is called, we allocate a page of memory and record the address of the allocated memory in the current process's `allocations` array.

In this commit, we don't map the allocated memory to the user's virtual address space. That is because, in our system, the heap area's user space virtual address is the same as the kernel's virtual address. We will fix this later.

- `malloc` and `free` [commit](https://github.com/taikiy/kernel/commit/c7b47a9c1139933fdedea99684122f22fc3f9dc0)
- Abstracting standard library system calls with `make_syscall` [commit](https://github.com/taikiy/kernel/commit/57ac430a09ce559101050ccc1010cbe4d97d95bb)
