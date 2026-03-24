cmake_minimum_required(VERSION 3.28)
project(examples C)

include(${LUNA_SOURCE_DIR}/cmake/options.cmake)
include(${LUNA_SOURCE_DIR}/cmake/findPackages.cmake)

findSDL3()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_HelloTriangle)
    add_executable(HelloTriangle helloTriangle.c)
    set_target_properties(HelloTriangle PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(HelloTriangle PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(HelloTriangle PRIVATE SDL3::SDL3 Luna)
endif ()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_HelloTriangleGeneratedPipeline)
    add_executable(HelloTriangleGeneratedPipeline PipelineGeneration/helloTriangle.c)
    set_target_properties(HelloTriangleGeneratedPipeline PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(HelloTriangleGeneratedPipeline PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(HelloTriangleGeneratedPipeline PRIVATE SDL3::SDL3 Luna)
endif ()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_HelloTriangleCompute)
    add_executable(HelloTriangleCompute helloTriangleCompute.c)
    set_target_properties(HelloTriangleCompute PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(HelloTriangleCompute PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(HelloTriangleCompute PRIVATE SDL3::SDL3 Luna)
endif ()

if (LUNA_EXAMPLE_ALL OR LUNA_EXAMPLE_LunaCube)
    configure_file(${LUNA_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)

    fetchLodePNG()
    fetchCglm()

    add_executable(LunaCube lunaCube.c)
    set_target_properties(LunaCube PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(LunaCube PRIVATE $<IF:$<CONFIG:Debug>,${DEBUG_FLAGS},${RELEASE_FLAGS}>)
    target_link_libraries(LunaCube PRIVATE LodePNG cglm SDL3::SDL3 Luna)
endif ()
