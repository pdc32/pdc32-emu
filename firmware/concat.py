import os
from intelhex import IntelHex

fw_filenames = [fn for fn in os.listdir(".") if fn.endswith(".hex")]

firmwares = []

for fn in fw_filenames:
    ih = IntelHex()
    ih.loadhex(fn)
    firmwares.append(ih)

out = []

for n in range(len(firmwares[0])):
    for j in range(3):
        out.append(chr(firmwares[2-j][n]))


q = "".join(out)
open("PDC32.firmware", "wb").write(q)
