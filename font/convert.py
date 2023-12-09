import os
from intelhex import IntelHex

def swap_bit_order(byte):
    return int(format(byte, '08b')[::-1], 2)

ih = IntelHex()
ih.loadhex("PDC32_ROM_ASCII.hex")

out = bytearray()
for byte_idx in range(256 * 16):
    out.append(swap_bit_order(ih[byte_idx]))

open("pdc32.font", "wb").write(out)

