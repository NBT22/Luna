cmake_minimum_required(VERSION 3.30)
project(examples C)

include(${CMAKE_SOURCE_DIR}/cmake/findPackages.cmake)

findVulkan()
findSDL3()

if (ALL OR HelloTriangle)
    add_executable(HelloTriangle helloTriangle.c)
    set_target_properties(HelloTriangle PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(HelloTriangle PRIVATE Vulkan::Vulkan SDL3::SDL3 ${CMAKE_BINARY_DIR}/libLuna.a)
    target_include_directories(HelloTriangle PRIVATE ${CMAKE_SOURCE_DIR}/include)
    add_dependencies(HelloTriangle Luna)
endif ()

if (ALL OR LunaCube)
    fetchLodepng()
    configure_file(${CMAKE_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)

    add_executable(LunaCube lunaCube.c ${LODEPNG_DIR}/lodepng.c)
    set_target_properties(LunaCube PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(LunaCube PRIVATE Vulkan::Vulkan SDL3::SDL3 ${CMAKE_BINARY_DIR}/libLuna.a)
    target_include_directories(LunaCube PRIVATE ${CMAKE_SOURCE_DIR}/include ${LODEPNG_DIR})
    add_dependencies(LunaCube Luna)
endif ()
