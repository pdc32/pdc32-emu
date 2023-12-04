import os
from intelhex import IntelHex

fw_filenames = ["Firmware_PDC32_B.hex", "Firmware_PDC32_C.hex", "Firmware_PDC32_A.hex"]
firmware_size_words = 32*1024;

firmwares = []

for fn in fw_filenames:
    ih = IntelHex()
    ih.loadhex(fn)
    firmwares.append(ih)

out = bytearray()

for n in range(firmware_size_words):
    for j in range(3):
        out.append(firmwares[j][n])

open("PDC32.firmware", "wb").write(out)
