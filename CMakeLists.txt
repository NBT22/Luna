cmake_minimum_required(VERSION 3.21)
project(Luna C CXX)

set(LUNA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "The root directory of the project")

include(cmake/options.cmake)
include(cmake/findPackages.cmake)

set(CMAKE_C_STANDARD 11)
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

add_library(Luna STATIC
        src/luna/core/Buffer.cpp
        src/luna/core/CommandPool.cpp
        src/luna/core/DescriptorSetLayout.cpp
        src/luna/core/Device.cpp
        src/luna/core/GraphicsPipeline.cpp
        src/luna/core/Image.cpp
        src/luna/core/Instance.cpp
        src/luna/core/Luna.cpp
        src/luna/core/RenderPass.cpp
        src/headers/luna/core/Buffer.hpp
        src/headers/luna/core/CommandBuffer.hpp
        src/headers/luna/core/CommandPool.hpp
        src/headers/luna/core/DescriptorSetLayout.hpp
        src/headers/luna/core/Device.hpp
        src/headers/luna/core/Fence.hpp
        src/headers/luna/core/GraphicsPipeline.hpp
        src/headers/luna/core/Image.hpp
        src/headers/luna/core/Instance.hpp
        src/headers/luna/core/Luna.hpp
        src/headers/luna/core/RenderPass.hpp
        src/headers/luna/core/Semaphore.hpp
        src/headers/luna/core/commandBuffer/CommandBuffer.hpp
        src/headers/luna/core/commandBuffer/CommandBufferArray.hpp
        src/headers/luna/implementations/core/Buffer.ipp
        src/headers/luna/implementations/core/CommandBuffer.ipp
        src/headers/luna/implementations/core/CommandPool.ipp
        src/headers/luna/implementations/core/DescriptorSetLayout.ipp
        src/headers/luna/implementations/core/Device.ipp
        src/headers/luna/implementations/core/Fence.ipp
        src/headers/luna/implementations/core/GraphicsPipeline.ipp
        src/headers/luna/implementations/core/Image.ipp
        src/headers/luna/implementations/core/Instance.ipp
        src/headers/luna/implementations/core/RenderPass.ipp
        src/headers/luna/implementations/core/Semaphore.ipp
        src/headers/luna/implementations/core/commandBuffer/CommandBuffer.ipp
        src/headers/luna/implementations/core/commandBuffer/CommandBufferArray.ipp
        include/luna/luna.h
        include/luna/lunaDevice.h
        include/luna/lunaDrawing.h
        include/luna/lunaImage.h
        include/luna/lunaInstance.h
        include/luna/lunaPipeline.h
        include/luna/lunaRenderPass.h
        include/luna/lunaTypes.h
)
target_compile_options(Luna PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
target_link_libraries(Luna PUBLIC _LunaInternalLibrary_Dependencies)
target_include_directories(Luna PUBLIC ${LUNA_SOURCE_DIR}/include)
target_include_directories(Luna PRIVATE ${LUNA_SOURCE_DIR}/src/headers)

if (LUNA_EXAMPLES)
    add_subdirectory(example)
endif ()
