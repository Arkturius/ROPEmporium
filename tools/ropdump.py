#!/usr/bin/python3

from os import *
from sys import *

ADDR_CODE = 0
ADDR_GADGET = 1
ADDR_PLT = 2
ADDR_GOT = 3
ADDR_HEAP = 4
ADDR_OTHER = 5
PADDING = 6

def exhaust_stdin():
    return stdin.buffer.read()

def ropdump(chain):
    rop = exhaust_stdin()
    chain.dump_grid()

class Block:
    
    def __init__(self, type, size, comment):
        self.type = type
        self.size = size
        self.comment = ""
        self.gadget = []

    def __str__(self):
        return repr(self)

    def __repr__(self):
        rep = f"{self.type} of size {self.size}"
        if self.comment != "":
            rep = f"[{self.comment}]: " + rep
        return rep

    def add_gadget(self, gadget):
        self.gadget = gadget

from collections import defaultdict

CHAR_SPACE = 0
CHAR_CORNER_TL = 1
CHAR_CORNER_TR = 2
CHAR_CORNER_BL = 3
CHAR_CORNER_BR = 4
CHAR_LINE_VERT = 5
CHAR_LINE_HORI = 6
CHAR_T_RIGHT   = 7
CHAR_T_LEFT    = 8
CHAR_T_DOWN    = 9
CHAR_T_UP      = 10
CHAR_CROSS     = 11

chars = [" ","┌","┐","└","┘","│","─","├","┤","┬","┴","┼"]

class Layout:

    def __init__(self, corner):
        self.blocks = []
        self.corner = corner

    def add_block(self, nb):
        self.blocks.append(nb)
        return self.blocks[-1]

class Chain:

    def __init__(self):
        self.layouts = []
        self.layout_corner = (0, 0)
        self.grid = [[CHAR_SPACE for i in range(160)] for j in range(48)]

    def dump_grid(self):
        for layout in self.layouts:
            if len(layout.blocks) != 0:
                cx, cy = layout.corner
                for i in range(13):
                    if i != 0 and i != 12: self.grid[cy][cx + i] = CHAR_LINE_HORI
                    else:
                        if i == 0: self.grid[cy][cx + i] = CHAR_CORNER_TL
                        else: self.grid[cy][cx + i] = CHAR_CORNER_TR
                for block in layout.blocks:
                    print(" block !")

        for i in range(48):
            for j in range(160):
                if self.grid[i][j] < 12:
                    print(chars[self.grid[i][j]], end="")
                else:
                    print(self.grid[i][j])
            print()

    def add_layout(self):
        self.layouts.append((Layout((self.layout_corner[0], self.layout_corner[1]))))
        self.layout_corner = (self.layout_corner[0] + 33, self.layout_corner[1])
        return self.layouts[-1]


BLOCK_ADDR = lambda type, comm: Block(type, 4, comm)

disp = Chain()

base_stack = disp.add_layout()
base_stack.add_block(Block(PADDING, ))

exploit = disp.add_layout()
exploit.add_block(Block(PADDING, 36, """   10570:	e92d4800 	push	{fp, lr}
   10574:	e28db004 	add	fp, sp, #4
   10578:	e24dd020 	sub	sp, sp, #32
   1057c:	e24b3024 	sub	r3, fp, #36	; 0x24"""))
exploit.add_block(BLOCK_ADDR(ADDR_CODE, "ret2win address"))

ropdump(disp)
