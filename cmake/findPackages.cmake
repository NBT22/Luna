include(FetchContent)
include(CheckIncludeFile)
include(${LUNA_SOURCE_DIR}/cmake/utils.cmake)

function(findDependencies)
    # TODO (0.3.0): Look into reducing the size of the slang binary
    if (LUNA_SLANG_SHADERS)
        runGenerator(SlangHeader)

        disableOptions(SLANG_ENABLE_CUDA SLANG_ENABLE_OPTIX SLANG_ENABLE_NVAPI SLANG_ENABLE_XLIB SLANG_ENABLE_AFTERMATH SLANG_ENABLE_DX_ON_VK SLANG_ENABLE_SLANG_RHI SLANG_ENABLE_DXIL SLANG_ENABLE_FULL_IR_VALIDATION SLANG_ENABLE_IR_BREAK_ALLOC SLANG_ENABLE_ASAN SLANG_ENABLE_COVERAGE SLANG_ENABLE_GFX SLANG_ENABLE_SLANGD SLANG_ENABLE_SLANGC SLANG_ENABLE_SLANGI SLANG_ENABLE_SLANG_GLSLANG SLANG_ENABLE_TESTS SLANG_ENABLE_EXAMPLES SLANG_ENABLE_REPLAYER)
        enableOptions(SLANG_USE_SYSTEM_VULKAN_HEADERS SLANG_USE_SYSTEM_GLSLANG SLANG_ENABLE_RELEASE_LTO)
        set(SLANG_LIB_TYPE STATIC CACHE STRING "")
        set(SLANG_SLANG_LLVM_FLAVOR DISABLE CACHE STRING "")
        FetchContent_Declare(
                slang
                GIT_REPOSITORY https://github.com/shader-slang/slang.git
                GIT_SUBMODULES external/spirv-headers external/miniz external/lz4 external/unordered_dense external/lua
                GIT_TAG v2025.23.1
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
                EXCLUDE_FROM_ALL
                SYSTEM
        )
        FetchContent_MakeAvailable(slang)

        target_link_libraries(Luna PRIVATE slang)
        target_compile_definitions(Luna PRIVATE LUNA_SLANG_SHADERS)
    endif ()

    makePackageAvailable(https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git v3.*.* VulkanMemoryAllocator CONFIG)

    find_package(Vulkan COMPONENTS volk QUIET)
    if (Vulkan_INCLUDE_DIRS STREQUAL "Vulkan_INCLUDE_DIR-NOTFOUND") # Unable to find Vulkan headers
        if (NOT DEFINED VULKAN_HEADERS_SOURCE_DIR)
            makePackageAvailable(https://github.com/KhronosGroup/Vulkan-Headers.git vulkan-sdk-1.4.*.* Headers)
            find_package(Vulkan COMPONENTS volk QUIET) # This is kept to check if volk is installed on the system
        endif ()
        set(Vulkan_INCLUDE_DIR ${VULKAN_HEADERS_SOURCE_DIR}/include)
        target_include_directories(Luna PUBLIC ${Vulkan_INCLUDE_DIR})
    endif ()
    if (Vulkan_FOUND) # Able to find Volk
        add_library(_LunaInternal_volk INTERFACE)
        target_link_libraries(_LunaInternal_volk INTERFACE Vulkan::volk)
        add_library(volk::volk_headers ALIAS _LunaInternal_volk)
    else () # No Vulkan installation
        set(VOLK_HEADERS_ONLY ON)
        set(VULKAN_HEADERS_INSTALL_DIR ${Vulkan_INCLUDE_DIR}/../)
        makePackageAvailable(https://github.com/zeux/volk.git vulkan-sdk-1.4.*.* volk)
    endif ()

    target_link_libraries(Luna PUBLIC volk::volk_headers GPUOpen::VulkanMemoryAllocator)

    if (LUNA_DEFINE_VK_NO_PROTOTYPES)
        target_compile_options(Luna PUBLIC $<IF:$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>,/DVK_NO_PROTOTYPES,-DVK_NO_PROTOTYPES>)
    endif ()
endfunction()
