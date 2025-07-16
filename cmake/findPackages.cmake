include(FetchContent)
include(CheckIncludeFile)

set(SYSTEM_DIRECTORY_FLAG $<IF:$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>,/external:I,-isystem>)

macro(getLatestPackageVersion gitRepo versionSplat)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} '${versionSplat}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LATEST_RELEASE)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} "${versionSplat}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE LATEST_RELEASE)
    endif ()
endmacro()

macro(makePackageAvailable gitRepo versionSplat packageName)
    getLatestPackageVersion(${gitRepo} ${versionSplat})

    FetchContent_Declare(
            ${packageName}
            GIT_REPOSITORY ${gitRepo}
            GIT_TAG ${LATEST_RELEASE}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
            FIND_PACKAGE_ARGS ${ARGN}
    )
    FetchContent_MakeAvailable(${packageName})
endmacro()

macro(findVulkan)
    find_package(Vulkan QUIET)
    add_library(VulkanLibrary INTERFACE)
    if (NOT Vulkan_FOUND)
        makePackageAvailable(https://github.com/KhronosGroup/Vulkan-Headers.git v1.4.* Headers)
        makePackageAvailable(https://github.com/KhronosGroup/Vulkan-Loader.git v1.4.* Loader)

        target_compile_options(VulkanLibrary INTERFACE "SHELL:${SYSTEM_DIRECTORY_FLAG} ${VULKAN_LOADER_SOURCE_DIR}/loader")
        target_link_libraries(VulkanLibrary INTERFACE Vulkan::Loader)
        target_include_directories(VulkanLibrary INTERFACE ${VULKAN_HEADERS_SOURCE_DIR}/include ${VULKAN_LOADER_SOURCE_DIR}/loader)
    else ()
        target_link_libraries(VulkanLibrary INTERFACE Vulkan::Vulkan)
        target_include_directories(VulkanLibrary INTERFACE ${Vulkan_INCLUDE_DIRS})
    endif ()
endmacro()

macro(findSDL3)
    makePackageAvailable(https://github.com/libsdl-org/SDL.git release-3.*.* SDL3 CONFIG)
endmacro()

macro(fetchVMA)
    check_include_file("vk_mem_alloc.h" VMA_FOUND)
    add_library(VMA INTERFACE)
    if (NOT VMA_FOUND)
        makePackageAvailable(https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git v3.*.* vma)
        target_compile_options(VMA INTERFACE "SHELL:${SYSTEM_DIRECTORY_FLAG} ${vma_SOURCE_DIR}/include")
        target_include_directories(VMA INTERFACE "${vma_SOURCE_DIR}/include")
    endif ()
endmacro()

macro(fetchCglm)
    check_include_file("cglm/cglm.h" cglm_FOUND)
    if (NOT cglm_FOUND)
        makePackageAvailable(https://github.com/recp/cglm.git v0.*.* cglm)
    else ()
        add_library(cglm INTERFACE)
    endif ()
endmacro()

macro(fetchLodePNG)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.cpp ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.c)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.h ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.h)
    add_library(LodePNG INTERFACE)
    target_compile_options(LodePNG INTERFACE "SHELL:${SYSTEM_DIRECTORY_FLAG} ${CMAKE_BINARY_DIR}/_deps/lodepng")
    target_sources(LodePNG INTERFACE "${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.c")
    target_include_directories(LodePNG INTERFACE "${CMAKE_BINARY_DIR}/_deps/lodepng")
endmacro()