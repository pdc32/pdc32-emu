import os
from intelhex import IntelHex

fw_filenames = ["1.5B/Firmware_PDC_32_B15B.hex", "1.5B/Firmware_PDC_32_C15B.hex", "1.5B/Firmware_PDC_32_A15B.hex"]
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
