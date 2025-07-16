/**
 * ROPDump.h | ROP chain visualizer.
 *
 * Do thid to use:
 *
 * #define ROPDUMP_IMPLEMENTATION
 * #include "ropdump.h"
 */

#ifndef _ROPDUMP_H
# define _ROPDUMP_H

# include <unistd.h>
# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>

# define	ROP_TODO	rop_error(__func__, "TODO")

typedef enum	rd_option
{
	RD_INIT			= 1 << 0,
	RD_OPENED		= 1 << 1,
	RD_STACK_OPENED = 1 << 2,
	RD_WORD_32		= 1 << 3,
	RD_WORD_64		= 1 << 4,
}	Option;

typedef enum	rd_addrtype
{
	ADDR_CODE,
	ADDR_GADGET,
	ADDR_PLT,
	ADDR_GOT,
	ADDR_HEAP,
	ADDR_OTHER,
	ADDR_STRING,
	ADDR_FLAG = 1UL << 63,
}	AddrType;

typedef struct	block_s
{
	union
	{
		uint64_t	size;
		uint64_t	addr_type;
	};
	union
	{
		uint64_t	value;
		void		*addr;
	};
	const char	*name;
	const char	**gadget;
}	Block;

typedef struct	chain_s
{
	uint64_t	size;
	uint64_t	capacity;
	Block		*layout;
	
	uint64_t	s_size;
	uint64_t	s_capacity;
	Block		*s_layout;

	uint8_t		*payload;
	uint8_t		*cursor;
	uint64_t	len;
	
	Option		options;
}	Chain;


/* Core functions *********************************************************** */

void
rop_begin(void);

void
rop_end(void);

void
rop_destroy(void);

Option
rop_opt_get(void);

void
rop_opt_set(Option opt);

void
rop_read_payload(void);

void
rop_dump();

/* Layout functions ********************************************************* */

void
rop_padding(uint32_t len);

void
rop_qword(void);

void
rop_dword(void);

void
rop_addr(AddrType type);

void
rop_string(const char *str);

/* Addons functions ********************************************************* */

void
rop_gadget(const char *name, ...);

void
rop_name(const char *name);

/* ************************************************************************** */

#endif // _ROPDUMP_H

#define ROPDUMP_IMPLEMENTATION
#ifdef ROPDUMP_IMPLEMENTATION

# define	RD_BLOCK_MIN	64
# define	RD_BLOCK_MAX	128

# define	COLOR_PADDING	"\x1b[38;2;70;70;70m"

const char	*addr_colors[ADDR_OTHER + 1] =
{
	[ADDR_CODE]		= "\x1b[38;2;0;122;204m",
	[ADDR_GADGET]	= "\x1b[38;2;162;89;255m",
	[ADDR_PLT]		= "\x1b[38;2;255;149;0m",
	[ADDR_GOT]		= "\x1b[38;2;255;213;0m",
	[ADDR_HEAP]		= "\x1b[38;2;61;220;151m",
	[ADDR_OTHER]	= "\x1b[38;2;110;117;130m"
};

Chain	rop = {0};

void
rop_begin(void)
{
	if (!(rop_opt_get() & RD_INIT))
	{
		printf("ropdump: RD_INIT not set.\n");
		exit(1);
	}

	rop.layout = (Block *) malloc(RD_BLOCK_MIN * sizeof(Block));
	if (!rop.layout)
	{
		printf("ropdump: rop.layout malloc failed.\n");
		exit(1);
	}

	memset(rop.layout, 0, RD_BLOCK_MIN * sizeof(Block));
	rop.size = 0;
	rop.capacity = RD_BLOCK_MIN;

	rop_read_payload();
	rop.cursor = rop.payload;

	rop_opt_set(rop_opt_get() | RD_OPENED);
}

void
rop_stack_begin(void)
{
	if (!(rop_opt_get() & RD_INIT))
	{
		printf("ropdump: RD_INIT not set.\n");
		exit(1);
	}

	rop.s_layout = (Block *) malloc(RD_BLOCK_MIN * sizeof(Block));
	if (!rop.s_layout)
	{
		printf("ropdump: rop.s_layout malloc failed.\n");
		exit(1);
	}

	memset(rop.s_layout, 0, RD_BLOCK_MIN * sizeof(Block));
	rop.s_size = 0;
	rop.s_capacity = RD_BLOCK_MIN;
	
	rop_opt_set(rop_opt_get() | RD_STACK_OPENED);
}

void
rop_end(void)
{
	rop_opt_set(rop_opt_get() & ~RD_OPENED);
}

__attribute__((destructor)) void
rop_destroy(void)
{
	if (rop.layout)
	{
		for (uint64_t i = 0; i < rop.size; ++i)
		{
			void	*name = (void *)rop.layout[i].name;
			if (name)
				free(name);
			rop.layout[i].name = NULL;
		}
		free(rop.layout);
		free(rop.payload);
		rop.layout = NULL;
		rop.payload = NULL;
	}
}

void
rop_stack_end(void)
{
	rop_opt_set(rop_opt_get() & ~RD_STACK_OPENED);
}

__attribute__((destructor)) void
rop_stack_destroy(void)
{
	if (rop.s_layout)
	{
		for (uint64_t i = 0; i < rop.s_size; ++i)
		{
			void	*name = (void *)rop.s_layout[i].name;
			if (name)
				free(name);
			rop.s_layout[i].name = NULL;
		}
		free(rop.s_layout);
		rop.s_layout = NULL;
	}
}

void
rop_error(const char *msg, const char *func)
{
	printf("ropdump: ");
	if (func)
		printf("%s: ", func);
	if (msg)
		printf("%s.", msg);
	printf("\n");
	exit(1);
}

void
rop_ready(void)
{
	if (!(rop_opt_get() & (RD_INIT | RD_OPENED)))
		rop_error("ropchain not in ready state", __func__);
}

void
rop_stack_ready(void)
{
	if (!(rop_opt_get() & (RD_INIT | RD_STACK_OPENED)))
		rop_error("stack not in ready state", __func__);
}

inline Option
rop_opt_get(void)
{
	return (rop.options);
}

inline void
rop_opt_set(Option opt)
{
	rop.options = opt;
}

void
rop_read_payload(void)
{
	int		r;

	rop.len = 0;
	rop.payload = NULL;
	do
	{
		rop.payload = realloc(rop.payload, (rop.len + 256) * sizeof(char));
		if (!rop.payload)
			rop_error("malloc failed", __func__);
		
		r = read(0, rop.payload + rop.len, 256);
		rop.len += r;
	}
	while (r == 256);
}

void
rop_dump_separator(uint32_t wsize)
{
	if (wsize == 4)
		printf("├─────────────┤\n");
	else
		printf("├─────────────────────────┤\n");
}

void
rop_dump(void)
{
	uint32_t	wsize = rop_opt_get() & RD_WORD_32 ? 4 : 8;

	rop.cursor = rop.payload;
	
	printf("\033[13C");

	if (wsize == 4)
		printf("┌─────────────┐\n");
	else
		printf("┌─────────────────────────┐\n");
	printf("\033[13C");
	for (uint64_t i = 0; i < rop.size; ++i)
	{
		Block		block = rop.layout[i];
		uint32_t	bsize = block.addr_type & ADDR_FLAG ? wsize : block.size;

		if (bsize & (wsize - 1))
			rop_error("block not word padded", __func__);

		if (block.addr_type & ADDR_FLAG)
		{
			if (i != 0)
			{
				rop_dump_separator(wsize);
				printf("\033[13C");
			}
			if (block.addr_type == (ADDR_FLAG | ADDR_STRING))
			{
				printf("│ %.15s │", (char *)block.addr);
			}
			else
			{
				if (wsize == 4)
					printf("│ %s0x%-8lx\033[0m  │", addr_colors[block.addr_type], block.value);
				else
					printf("│ %s0x%-16lx\033[0m      │", addr_colors[block.addr_type], block.value);
			}
			if (block.name)
				printf(" <- %s", block.name);
			if (block.gadget)
			{
				const char	**tmp = block.gadget;
				while (*tmp)
				{
					printf("[ %s%s\033[0m ]", addr_colors[ADDR_GADGET], *tmp);
					tmp++;
				}
			}
			printf("\n");
			printf("\033[13C");
			rop.cursor += wsize;
		}
		else
		{
			if (i && (rop.layout[i - 1].addr_type & ADDR_FLAG))
			{
				rop_dump_separator(wsize);
				printf("\033[13C");
			}
			for (uint32_t j = 0; j < bsize; ++j)
			{
				if (j % wsize == 0)
					printf("│ %s", block.size > wsize ? COLOR_PADDING: "");
				printf("%s%02x", j % wsize ? " " : "", *rop.cursor++);
				if (j % wsize == wsize - 1)
				{
					printf("\033[0m │");
					if (j == wsize - 1 && block.name)
						printf(" <- %s", block.name);
					printf("\n");
					printf("\033[13C");
				}
			}
		}
	}
	if (wsize == 4)
		printf("└─────────────┘\n");
	else
		printf("└─────────────────────────┘\n");
}

/* ************************************************************************** */

void
rop_block(uint64_t size, uint64_t value)
{
	Block	new = (Block) { .size = size, .value = value };

	if (rop.size + 1 == rop.capacity)
		rop_error("ropchain too long.", __func__);

	rop.layout[rop.size++] = new;
}

uint64_t
rop_read_addr(uint32_t wsize)
{
	uint64_t	addr;

	if (wsize == 4)
		addr = (uint64_t)*(uint32_t *)rop.cursor;
	else
		addr = *(uint64_t *)rop.cursor;
	rop.cursor += wsize;
	return (addr);
}

void
rop_padding(uint32_t len)
{
	rop_ready();
	rop_block(len, '\x00');
	rop.cursor += len;
}

void
rop_qword(void)
{
	rop_ready();
	rop_block(8, rop_read_addr(8));
}

void
rop_dword(void)
{
	rop_ready();
	rop_block(4, rop_read_addr(4));
}

void
rop_addr(AddrType type)
{
	rop_ready();

	uint32_t	wsize = rop_opt_get() & RD_WORD_32 ? 4 : 8;

	rop_block(ADDR_FLAG | type, rop_read_addr(wsize));
}

void
rop_string(const char *str)
{
	rop_ready();

	rop_block(ADDR_FLAG | ADDR_STRING, (uint64_t)str);
}

/* ************************************************************************** */

void
rop_name(const char *name)
{
	rop_ready();

	if (rop.size == 0)
		rop_error("no last block", __func__);

	Block	*last = &rop.layout[rop.size - 1];
	
	last->name = strdup(name);
	if (!last->name)
		rop_error("malloc failed", __func__);
}

void
rop_gadget(const char *first, ...)
{
	rop_ready();

	if (rop.size == 0)
		rop_error("no last block", __func__);

	const char	*instr;
	uint32_t	count = 1;
	va_list		instrs;
	va_list		tmp;

	va_start(instrs, first);

	va_copy(tmp, instrs);
	do { instr = va_arg(tmp, const char *); count++; }
	while (instr);
	va_end(tmp);

	Block	*last = &rop.layout[rop.size - 1];

	last->gadget = malloc(count * sizeof(char *));
	if (!last->gadget)
		rop_error("malloc failed", __func__);
	memset(last->gadget, 0, count * sizeof(char *));
	last->gadget[0] = strdup(first);
	for (uint32_t i = 1; i < count - 1; ++i)
	{
		instr = va_arg(instrs, const char *);
		if (instr)
			last->gadget[i] = strdup(instr);
	}
	va_end(instrs);
}

#endif // ROPDUMP_IMPLEMENTATION
