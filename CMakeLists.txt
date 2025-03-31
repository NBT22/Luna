cmake_minimum_required(VERSION 3.30)
project(Luna CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -isystem ${CMAKE_SOURCE_DIR}/vma/include")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections")

find_package(Vulkan REQUIRED) # 3d
include_directories(${Vulkan_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE PRIVATE_HEADERS "src/headers/luna/*.hpp")
file(GLOB_RECURSE PRIVATE_IMPLEMENTATIONS "src/headers/luna/implementations/*.ipp")
file(GLOB_RECURSE PUBLIC_HEADERS "include/luna/*.h")

add_subdirectory(vma)
add_library(Luna STATIC ${SOURCES} ${PRIVATE_HEADERS} ${PRIVATE_IMPLEMENTATIONS} ${PUBLIC_HEADERS})
target_link_libraries(Luna ${Vulkan_LIBRARIES} GPUOpen::VulkanMemoryAllocator)
include_directories(Luna PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
include_directories(Luna PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_subdirectory(example)
