# ret2win

## x86_64: (TODO)

## x86: (TODO)

## ARM:
```asm
00010570 <pwnme>:
   10570:	e92d4800 	push	{fp, lr}
   10574:	e28db004 	add	fp, sp, #4
   10578:	e24dd020 	sub	sp, sp, #32
   ...
   105b0:	e24b3024 	sub	r3, fp, #36	; 0x24
   105b4:	e3a02038 	mov	r2, #56	; 0x38
   105b8:	e1a01003 	mov	r1, r3
   105bc:	e3a00000 	mov	r0, #0
   105c0:	ebffff80 	bl	103c8 <read@plt>
   ...
   105d0:	e24bd004 	sub	sp, fp, #4
   105d4:	e8bd8800 	pop	{fp, pc}
```
The `pwnme` function prologue declares 32 bytes of stack.
The read function is then used to get 56 characters from user input.

We can deduce the address where those characters will be placed.
`fp - 36` is equivalent to `sp`. since the frame pointer is stored right under the 32 bytes buffer.

Since `fp` and `pc` are popped in this order at the end, if we want to overwrite `pc`,
we need to write 36 bytes beyond the buffer

The `ret2win` function is at address `0x000105ec` so our final buffer is:
36 bytes of padding + ret2win address


```asm
000105ec <ret2win>:
   105ec:	e92d4800 	push	{fp, lr}
   ...
```
