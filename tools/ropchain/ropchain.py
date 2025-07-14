#!/usr/bin/python3

from sys import stdout

def pack8(w):
    return w.to_bytes(8, 'little')

def pack4(w):
    return w.to_bytes(4, 'little')

def packS(s, align):
    return bytes(s + (((len(s) & ~(align - 1) - 1) + align) - len(s)) * "\x00", "utf-8")

def raw(stream):
    return "".join([f"\\x{b:02x}" for b in stream])

def echo(byte_stream):
    stdout.buffer.write(byte_stream)
    
def pad(n, c = b" "):
    return c * n
