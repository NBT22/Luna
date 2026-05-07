# TODO: Test version
#  Slang requires 3.22
#  EXCLUDE_FROM_ALL requires 3.28
cmake_minimum_required(VERSION 3.28)
project(Luna C CXX)

set(LUNA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "The root directory of the project")

include(cmake/options.cmake)
include(cmake/findPackages.cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

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

add_library(Luna STATIC
        src/Buffer.cpp
        src/CommandBuffer.cpp
        src/CommandPool.cpp
        src/ComputePipeline.cpp
        src/DescriptorSetLayout.cpp
        src/Device.cpp
        src/Fence.cpp
        src/GraphicsPipeline.cpp
        src/Image.cpp
        src/Instance.cpp
        src/Luna.cpp
        src/RenderPass.cpp
        src/ShaderModule.cpp
        src/Semaphore.cpp

        include/internal/helpers/Handle.hpp
        include/internal/helpers/Pipeline.hpp
        include/internal/ComputePipeline.hpp
        include/internal/Buffer.hpp
        include/internal/CommandBuffer.hpp
        include/internal/CommandPool.hpp
        include/internal/DescriptorSetLayout.hpp
        include/internal/Device.hpp
        include/internal/Fence.hpp
        include/internal/GraphicsPipeline.hpp
        include/internal/Image.hpp
        include/internal/Instance.hpp
        include/internal/Luna.hpp
        include/internal/RenderPass.hpp
        include/internal/Semaphore.hpp
        include/internal/ShaderModule.hpp
        include/internal/SlangSession.hpp

        include/bindings/C/luna/luna.h
        include/bindings/C/luna/lunaBuffer.h
        include/bindings/C/luna/lunaCommandBuffer.h
        include/bindings/C/luna/lunaDevice.h
        include/bindings/C/luna/lunaDrawing.h
        include/bindings/C/luna/lunaImage.h
        include/bindings/C/luna/lunaInstance.h
        include/bindings/C/luna/lunaSynchronization.h
        include/bindings/C/luna/lunaTypes.h
)
target_precompile_headers(Luna PRIVATE
        include/internal/helpers/Handle.hpp
        include/internal/helpers/Pipeline.hpp
        include/internal/ComputePipeline.hpp
        include/internal/Buffer.hpp
        include/internal/CommandBuffer.hpp
        include/internal/CommandPool.hpp
        include/internal/DescriptorSetLayout.hpp
        include/internal/Device.hpp
        include/internal/Fence.hpp
        include/internal/GraphicsPipeline.hpp
        include/internal/Image.hpp
        include/internal/Instance.hpp
        include/internal/Luna.hpp
        include/internal/RenderPass.hpp
        include/internal/Semaphore.hpp
        include/internal/ShaderModule.hpp
        include/internal/SlangSession.hpp
)
target_compile_options(Luna PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
target_include_directories(Luna PUBLIC ${LUNA_SOURCE_DIR}/include/bindings/C)
target_include_directories(Luna PRIVATE ${LUNA_SOURCE_DIR}/include/internal ${LUNA_SOURCE_DIR}/generated/include)

findDependencies()

if (LUNA_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IS_IPO_SUPPORTED)
    if (IS_IPO_SUPPORTED)
        set_target_properties(Luna PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    else ()
        message(WARNING "LTO option enabled, but LTO/IPO is not supported!")
    endif ()
endif ()

if (LUNA_EXAMPLES)
    add_subdirectory(example)
endif ()
