if (NOT TARGET _LunaInternal_compileOptions)
    add_library(_LunaInternal_compileOptions INTERFACE)
endif ()

function(compileOption variable helpText value)
    option(${variable} ${helpText} ${value})
    if (${${variable}})
        target_compile_definitions(_LunaInternal_compileOptions INTERFACE ${variable})
    endif ()
endfunction()

option(LUNA_USE_PIPES "Enable the usage of pipes in GCC, decreasing compile time at the cost of higher RAM usage when compiling" ON)
option(LUNA_ENABLE_LTO "Enable LTO on release builds, which can increase performance at the cost of slower compile time and increased RAM usage when compiling" ON)

option(LUNA_WARNINGS_ARE_FATAL "Treat warnings as errors, causing the build to fail if any warnings are present" OFF)

option(LUNA_DEFINE_VK_NO_PROTOTYPES "Define the `VK_NO_PROTOTYPES` macro, which allows the application to include <vulkan/vulkan_core.h> instead of <volk.h>" ON)
compileOption(LUNA_SLANG_SHADERS "Enable compilation and reflection of slang shader modules. Increases both configure and first compile time significantly" OFF)

option(LUNA_EXAMPLES "Enable building of example projects" ${PROJECT_IS_TOP_LEVEL})
option(LUNA_EXAMPLE_ALL "Enable all example project targets. Note that this overrides the settings of the individual projects" ON)
option(LUNA_EXAMPLE_HelloTriangle "Enable the HelloTriangle example project target" OFF)
option(LUNA_EXAMPLE_HelloTriangleCompute "Enable the HelloTriangleCompute example project target" OFF)
option(LUNA_EXAMPLE_LunaCube "Enable the LunaCube example project target" OFF)
