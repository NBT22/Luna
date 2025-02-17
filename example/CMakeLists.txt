cmake_minimum_required(VERSION 3.30)
project(example C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wno-discarded-qualifiers -fvisibility=hidden")

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections")

find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIR})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(example main.c)
target_link_libraries(example ${SDL3_LIBRARY} ${Vulkan_LIBRARIES} SDL3 ${CMAKE_SOURCE_DIR}/cmake-build-debug/libLuna.a)
set_target_properties(example PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(example Luna)
