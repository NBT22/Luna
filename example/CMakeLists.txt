cmake_minimum_required(VERSION 3.30)
project(examples C)

include(${CMAKE_SOURCE_DIR}/cmake/options.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/findPackages.cmake)

findSDL3()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_HelloTriangle)
    add_executable(HelloTriangle helloTriangle.c)
    set_target_properties(HelloTriangle PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(HelloTriangle PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(HelloTriangle PRIVATE SDL3::SDL3 Luna)
endif ()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_LunaCube)
    configure_file(${CMAKE_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)

    fetchLodePNG()
    fetchCglm()

    add_executable(LunaCube lunaCube.c)
    set_target_properties(LunaCube PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(LunaCube PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(LunaCube PRIVATE LodePNG cglm SDL3::SDL3 Luna)
endif ()
