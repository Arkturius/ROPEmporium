# ROPEmporium

This repo contains my solutions for the 8 challenges of [ROPEmporium](https://ropemporium.com/).   
As said on the website:
```
The ROP Emporium challenges attempt to remove as much reliance on reverse-engineering   
and bug hunting as possible so you can focus on building your ROP chains.   
Each binary has the same vulnerability:   
a user-provided string is copied into a stack-based buffer with no bounds checking,   
allowing a function’s saved return address to be overwritten.
```   
So my walkthroughs will focus on ROP chain building rather than re-explaining the vulnerability.

Current solving status:
```
    x86: Solved all ✅
 x86_64: Solved all ✅
  ARMv5: Solved all ✅
   MIPS: WIP...     ⚙
```
## 0 - ret2win
Overwrite a return addres from the vulnerable function.

## 1 - split
First ROP chain, elements from the first challenge have been split apart.

## 2 - callme
Redirect execution to call 3 functions in a certain order, with custom arguments.

## 3 - write4
Explore gadgets to find a "write-where-what", allowing us to arbitrary write into memory.
Use it inside a ROP chain.

## 4 - badchars
Learn to deal with "bad characters" that would be removed from the user input.

## 5 - fluff
Achieve black wizardry with some "questionableGadgets"

## 6 - pivot
Learn to redirect the stack pointer to get more working space.

## 7 - ret2csu
Universal ROP exploit, using gadgets inside the GCC-generated function `__libc_csu_init`
