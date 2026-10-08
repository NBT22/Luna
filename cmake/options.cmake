option(LUNA_USE_PIPES "Enable the usage of pipes in GCC, decreasing compile time at the cost of higher RAM usage when compiling" ON)
option(LUNA_ENABLE_LTO "Enable LTO on release builds, which can increase performance at the cost of slower compile time and increased RAM usage when compiling" ON)

option(LUNA_WARNINGS_ARE_FATAL "Treat warnings as errors, causing the build to fail if any warnings are present" OFF)

option(LUNA_DEFINE_VK_NO_PROTOTYPES "Define the `VK_NO_PROTOTYPES` macro, which allows the application to include <vulkan/vulkan_core.h> instead of <volk.h>" ON)
option(LUNA_SLANG_SHADERS "Enable compilation and reflection of slang shader modules. Increases both configure and first compile time significantly" OFF)
