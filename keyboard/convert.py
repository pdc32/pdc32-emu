
def bytely(string):
    response = []
    string = "".join(chr(int(s, 16)) for s in string.split(","))
    for b in string:
        #if 32 < ord(b) < 128:
            #response.append(b)
        #else:
            response.append('\\x%02x' % (ord(b)))
    return '"' + "".join(response) + '"'

print("""
#include <unordered_map>
#include <SDL2/SDL.h>

struct PS2_scancode {
    const char* key_make;
    const char* key_break;
};

const std::unordered_map<SDL_Scancode, PS2_scancode> ps2_map = {
""")


lines = open("mapping.tsv").readlines()
for l in lines:
    scan, key_make, key_break = l.split("\t")
    # print(scan, key_make, key_break)
    print("    { %s, {%s, %s} }," % (
        "SDL_SCANCODE_" + scan.upper(),
        bytely(key_make),
        bytely(key_break.strip())
    ))

print("""
};
""")
