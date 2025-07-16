include(FetchContent)
include(CheckIncludeFile)

function(getLatestPackageVersion gitRepo versionSplat)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} '${versionSplat}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE LATEST_RELEASE)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} "${versionSplat}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE LATEST_RELEASE)
    endif ()
    set(LATEST_RELEASE ${LATEST_RELEASE} PARENT_SCOPE)
endfunction()

function(findVulkanHeaders)
    find_package(Vulkan QUIET)
    if (NOT Vulkan_FOUND)
        getLatestPackageVersion(https://github.com/KhronosGroup/Vulkan-Headers.git v1.4.*)

        FetchContent_Declare(
                Headers
                GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(Headers)
        set(Vulkan_INCLUDE_DIRS "${VULKAN_HEADERS_SOURCE_DIR}/include")
    endif ()
    set(Vulkan_INCLUDE_DIRS "${Vulkan_INCLUDE_DIRS}" PARENT_SCOPE)
endfunction()

function(findVulkan)
    find_package(Vulkan QUIET)
    add_library(VulkanLibrary INTERFACE)
    if (NOT Vulkan_FOUND)
        getLatestPackageVersion(https://github.com/KhronosGroup/Vulkan-Headers.git v1.4.*)

        FetchContent_Declare(
                Headers
                GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_Declare(
                Loader
                GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(Headers Loader)

        target_include_directories(VulkanLibrary INTERFACE ${VULKAN_HEADERS_SOURCE_DIR}/include ${VULKAN_LOADER_SOURCE_DIR}/loader)
        target_link_libraries(VulkanLibrary INTERFACE Vulkan::Loader)
    else ()
        target_include_directories(VulkanLibrary INTERFACE ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(VulkanLibrary INTERFACE Vulkan::Vulkan)
    endif ()
endfunction()

function(findSDL3)
    find_package(SDL3 CONFIG QUIET)
    if (NOT SDL3_FOUND)
        getLatestPackageVersion(https://github.com/libsdl-org/SDL.git release-3.*.*)

        FetchContent_Declare(
                SDL3
                GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(SDL3)
    endif ()
endfunction()

function(fetchCglm)
    check_include_file("cglm/cglm.h" cglm_FOUND)
    if (NOT cglm_FOUND)
        getLatestPackageVersion(https://github.com/recp/cglm.git v0.*.*)

        FetchContent_Declare(
                cglm
                GIT_REPOSITORY https://github.com/recp/cglm.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(cglm)
    else()
        add_library(cglm INTERFACE)
    endif ()
endfunction()

function(fetchLodepng)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.cpp ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.c)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.h ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.h)
    set(LODEPNG_DIR "${CMAKE_BINARY_DIR}/_deps/lodepng" PARENT_SCOPE)
endfunction()