# TODO: Test version
#  Slang requires 3.22
#  EXCLUDE_FROM_ALL requires 3.28
cmake_minimum_required(VERSION 3.22)
project(Luna C CXX)

set(LUNA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "The root directory of the project")

include(cmake/options.cmake)
include(cmake/findPackages.cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

set(LUNA_USE_LTO $<AND:$<BOOL:LUNA_ENABLE_LTO>,$<CONFIG:Release>>)
if (LUNA_USE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IS_IPO_SUPPORTED)
    if (IS_IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message(WARNING "LTO option enabled, but LTO/IPO is not supported!")
    endif ()
endif ()

string(REPLACE " " ";" COMMON_FLAGS $<$<COMPILE_LANGUAGE:C>:${CMAKE_C_FLAGS}>$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CXX_FLAGS}>)
string(REPLACE " " ";" DEBUG_FLAGS $<$<COMPILE_LANGUAGE:C>:${CMAKE_C_FLAGS_DEBUG}>$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CXX_FLAGS_DEBUG}>)
string(REPLACE " " ";" RELEASE_FLAGS $<$<COMPILE_LANGUAGE:C>:${CMAKE_C_FLAGS_RELEASE}>$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CXX_FLAGS_RELEASE}>)
if (MSVC)
    set(COMMON_FLAGS /W4 $<$<BOOL:${LUNA_WARNINGS_ARE_FATAL}>:/WX> /wd4244 /wd4267 /wd4146 /external:W0 ${COMMON_FLAGS})
    set(DEBUG_FLAGS ${COMMON_FLAGS} ${DEBUG_FLAGS} /Zi /Od)
    set(RELEASE_FLAGS ${COMMON_FLAGS} ${RELEASE_FLAGS} /O2 /DNDEBUG)
else ()
    set(COMMON_FLAGS $<$<BOOL:${LUNA_USE_PIPES}>:-pipe> -Wpedantic -Wall -Wextra $<$<BOOL:${LUNA_WARNINGS_ARE_FATAL}>:-Werror> -Wno-missing-field-initializers ${COMMON_FLAGS})
    set(DEBUG_FLAGS ${COMMON_FLAGS} ${DEBUG_FLAGS} -g -O0)
    set(RELEASE_FLAGS ${COMMON_FLAGS} ${RELEASE_FLAGS} -O3 -DNDEBUG)
endif ()

findDependencies()

# TODO: The scripts to generate the header should probably live elsewhere
if (WIN32)
    add_custom_target(GenerateShaderHeader ALL BYPRODUCTS COMMAND powershell -ExecutionPolicy RemoteSigned -NoProfile -NonInteractive ${LUNA_SOURCE_DIR}/src/shaders/GenerateHeader.ps1 WORKING_DIRECTORY ${LUNA_SOURCE_DIR}/src/shaders)
else ()
    add_custom_target(GenerateShaderHeader ALL BYPRODUCTS COMMAND sh ${LUNA_SOURCE_DIR}/src/shaders/GenerateHeader.sh WORKING_DIRECTORY ${LUNA_SOURCE_DIR}/src/shaders)
endif ()

add_library(Luna STATIC
        src/Buffer.cpp
        src/CommandPool.cpp
        src/ComputePipeline.cpp
        src/DescriptorSetLayout.cpp
        src/Device.cpp
        src/GraphicsPipeline.cpp
        src/Image.cpp
        src/Instance.cpp
        src/Luna.cpp
        src/RenderPass.cpp
        src/ShaderModule.cpp

        src/headers/helpers/Handle.hpp
        src/headers/helpers/Pipeline.hpp
        src/headers/commandBuffer/CommandBuffer.hpp
        src/headers/commandBuffer/CommandBufferArray.hpp
        src/headers/ComputePipeline.hpp
        src/headers/Buffer.hpp
        src/headers/CommandBuffer.hpp
        src/headers/CommandPool.hpp
        src/headers/DescriptorSetLayout.hpp
        src/headers/Device.hpp
        src/headers/Fence.hpp
        src/headers/GraphicsPipeline.hpp
        src/headers/Image.hpp
        src/headers/Instance.hpp
        src/headers/Luna.hpp
        src/headers/RenderPass.hpp
        src/headers/Semaphore.hpp
        src/headers/ShaderModule.hpp
        src/headers/SlangSession.hpp

        include/luna/luna.h
        include/luna/lunaBuffer.h
        include/luna/lunaDevice.h
        include/luna/lunaDrawing.h
        include/luna/lunaImage.h
        include/luna/lunaInstance.h
        include/luna/lunaTypes.h
)
add_dependencies(Luna GenerateShaderHeader)
target_compile_options(Luna PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
target_link_libraries(Luna PUBLIC _LunaInternal_PublicDependencies)
target_link_libraries(Luna PRIVATE _LunaInternal_PrivateDependencies _LunaInternal_compileOptions)
target_include_directories(Luna PUBLIC ${LUNA_SOURCE_DIR}/include)
target_include_directories(Luna PRIVATE ${LUNA_SOURCE_DIR}/src/headers ${LUNA_SOURCE_DIR}/src/shaders)

if (LUNA_EXAMPLES)
    add_subdirectory(example)
endif ()
