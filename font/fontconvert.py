source_fontname = "Bm437_Trident_8x16.FON"
dest_fontname = "pdc32.font"

def dump(fn):
    content = open(fn, "rb").read()
    for n, c in enumerate(content):
        b = '{:08b}'.format(ord(c))
        r = b.replace("0", " ").replace("1", "X")
        p = '{:04x} | {}'.format(n, r)
        print(p)

def convert(fn_in, fn_out):
    content = open(fn_in, "rb").read()
    content = content[0x65a:0x65a+16*255]
    signature = b"".join( chr(c) for c in [
        0b11100000,
        0b10010000,
        0b11110000,
        0b10010110,
        0b11101001,
        0b00001000,
        0b01101001,
        0b10000110,
        0b10110000,
        0b10010000,
        0b01110001,
        0b00010001,
        0b00010101,
        0b00010101,
        0b00001010,
        0b00000000,
    ])
    content = content + signature
    open(fn_out, "wb").write(content)

convert(source_fontname, dest_fontname)
dump(dest_fontname)
