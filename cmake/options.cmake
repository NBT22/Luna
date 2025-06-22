option(USE_PIPES "Enable the usage of pipes in GCC, decreasing compile time at the cost of higher RAM usage when compiling" ON)
option(ENABLE_LTO "Enable LTO on release builds, which can increase performance at the cost of slower compile time and increased RAM usage when compiling" ON)

option(EXAMPLES "Enable building of example projects" ON)
option(ALL "Enable all example project targets" ON)
option(HelloTriangle "Enable the HelloTriangle example project target" ON)
option(LunaCube "Enable the LunaCube example project target" ON)
