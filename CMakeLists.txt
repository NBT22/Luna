cmake_minimum_required(VERSION 3.30)
project(Luna CXX)

include(cmake/options.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IS_IPO_SUPPORTED)
    if (IS_IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else ()
        message(WARNING "LTO option enabled, but LTO/IPO is not supported!")
    endif ()
endif ()

set(COMMON_FLAGS "-Wpedantic -Wall -Wextra -Wno-missing-field-initializers")
if (USE_PIPES)
    set(COMMON_FLAGS "-pipe ${COMMON_FLAGS}")
endif ()

set(CMAKE_C_FLAGS "${COMMON_FLAGS} ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${COMMON_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${COMMON_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -g -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ${CMAKE_CXX_FLAGS} -isystem ${CMAKE_SOURCE_DIR}/vma/include")
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(vma)
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
        src/headers/luna/core/CommandBuffer/CommandBuffer.hpp
        src/headers/luna/core/CommandBuffer/CommandBufferArray.hpp
        src/headers/luna/implementations/core/Buffer.ipp
        src/headers/luna/implementations/core/CommandPool.ipp
        src/headers/luna/implementations/core/DescriptorSetLayout.ipp
        src/headers/luna/implementations/core/Device.ipp
        src/headers/luna/implementations/core/Fence.ipp
        src/headers/luna/implementations/core/GraphicsPipeline.ipp
        src/headers/luna/implementations/core/Image.ipp
        src/headers/luna/implementations/core/Instance.ipp
        src/headers/luna/implementations/core/RenderPass.ipp
        src/headers/luna/implementations/core/Semaphore.ipp
        src/headers/luna/implementations/core/CommandBuffer/CommandBuffer.ipp
        src/headers/luna/implementations/core/CommandBuffer/CommandBufferArray.ipp
        include/luna/luna.h
        include/luna/lunaDevice.h
        include/luna/lunaDrawing.h
        include/luna/lunaImage.h
        include/luna/lunaInstance.h
        include/luna/lunaPipeline.h
        include/luna/lunaRenderPass.h
        include/luna/lunaTypes.h
)
target_include_directories(Luna PRIVATE ${CMAKE_SOURCE_DIR}/src/headers ${CMAKE_SOURCE_DIR}/include)

if (EXAMPLES)
    add_subdirectory(example)
endif ()
