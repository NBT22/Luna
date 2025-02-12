cmake_minimum_required(VERSION 3.30)
project(Luna CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -DNDEBUG -Wl,--gc-sections -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -Wl,--gc-sections -fdata-sections -ffunction-sections")

find_package(Vulkan REQUIRED) # 3d
include_directories(${Vulkan_INCLUDE_DIRS})

#file(GLOB_RECURSE SOURCES "src/*.cpp")
#file(GLOB_RECURSE PRIVATE_HEADERS "src/Headers/luna/*.h")
file(GLOB_RECURSE PUBLIC_HEADERS "include/luna/*.h")
#file(GLOB_RECURSE PRIVATE_HEADERS "src/Headers/luna/helpers/LockingList.h")

#add_library(Luna STATIC ${SOURCES} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})
add_executable(Luna src/luna.cpp src/Headers/luna/helpers/LockingList.h src/Headers/luna/implementations/LockingList.tpp ${PUBLIC_HEADERS})
include_directories(Luna PUBLIC "${CMAKE_SOURCE_DIR}/include")
include_directories(Luna PUBLIC "${CMAKE_SOURCE_DIR}/src/Headers")
