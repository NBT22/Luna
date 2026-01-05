echo '#pragma once' > Shaders.hpp
{
  echo '#ifdef LUNA_SLANG_SHADERS'
  echo 'const char LUNA_SLANG_HEADER[] = {'
  hexdump -v -e '/1 "0x%02X, "' Luna.slang | head -c -2
  echo '};'
  echo '#endif'
} >> Shaders.hpp
