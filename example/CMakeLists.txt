cmake_minimum_required(VERSION 3.30)
project(examples C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Wno-discarded-qualifiers -fvisibility=hidden")

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -s -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections")

find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIR})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

find_package(lodepng-c CONFIG REQUIRED)

configure_file(${CMAKE_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)

add_executable(HelloTriangle helloTriangle.c)
target_link_libraries(HelloTriangle ${SDL3_LIBRARY} ${Vulkan_LIBRARIES} SDL3 ${CMAKE_BINARY_DIR}/libLuna.a)
set_target_properties(HelloTriangle PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(HelloTriangle Luna)

add_executable(LunaCube lunaCube.c)
target_link_libraries(LunaCube ${SDL3_LIBRARY} ${Vulkan_LIBRARIES} lodepng-c SDL3 ${CMAKE_BINARY_DIR}/libLuna.a)
set_target_properties(LunaCube PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(LunaCube Luna)
