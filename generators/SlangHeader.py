import os

with open('SlangHeader.slang', 'r') as slang:
    os.makedirs(name='../generated', exist_ok=True)
    with open('../generated/SlangHeader.hpp', 'w+') as header:
        header.write("#pragma once\n#ifdef LUNA_SLANG_SHADERS\nconstexpr char LUNA_SLANG_HEADER[] = R\"(" +
                     slang.read() +
                     ")\";\n#endif")
