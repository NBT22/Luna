'#pragma once' > Shaders.hpp
'#ifdef LUNA_SLANG_SHADERS' >> Shaders.hpp
'const char LUNA_SLANG_HEADER[] = {' + (([System.IO.File]::ReadAllBytes("Luna.slang") | foreach { '0x' + "{0:X2}" -f $_ }) -join ', ') + '};' >> Shaders.hpp
'#endif' >> Shaders.hpp
