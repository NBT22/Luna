option(LUNA_USE_PIPES "Enable the usage of pipes in GCC, decreasing compile time at the cost of higher RAM usage when compiling" ON)
option(LUNA_ENABLE_LTO "Enable LTO on release builds, which can increase performance at the cost of slower compile time and increased RAM usage when compiling" ON)

option(LUNA_WARNINGS_ARE_FATAL "Treat warnings as errors, causing the build to fail if any warnings are present" ON)

option(LUNA_DEFINE_VK_NO_PROTOTYPES "Define the `VK_NO_PROTOTYPES` macro, which allows the application to include <vulkan/vulkan_core.h> instead of <volk.h>" ON)

option(LUNA_EXAMPLES "Enable building of example projects" OFF)
option(LUNA_EXAMPLE_ALL "Enable all example project targets" ON)
option(LUNA_EXAMPLE_HelloTriangle "Enable the HelloTriangle example project target" ON)
option(LUNA_EXAMPLE_LunaCube "Enable the LunaCube example project target" ON)
