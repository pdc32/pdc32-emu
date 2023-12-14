import os
from intelhex import IntelHex

def swap_bit_order(byte):
    return int(format(byte, '08b')[::-1], 2)

ih = IntelHex()
ih.loadhex("PDC_Files_2.hex")

out = bytearray()
for byte_idx in range(len(ih)):
    out.append(swap_bit_order(ih[byte_idx]))

open("PDC_Files.bin", "wb").write(out)

