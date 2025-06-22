include(FetchContent)

function(findVulkan)
    find_package(Vulkan)
    if (NOT Vulkan_FOUND)
        find_package(Git >=2.18 REQUIRED)
        # TODO: Windows support - The git command should work, but I don't think tail or cut exist
        execute_process(COMMAND ${CMAKE_COMMAND} -E ${GIT_EXECUTABLE} -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort='version:refname' --tags https://github.com/KhronosGroup/Vulkan-Headers.git 'v1.4.*' | tail --lines=1 | cut --delimiter='/' --fields=3 OUTPUT_VARIABLE LATEST_RELEASE)

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
        FetchContent_Declare(
                Validation
                GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-ValidationLayers.git
                GIT_TAG ${LATEST_RELEASE}
                GIT_SHALLOW TRUE
                GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(Headers, Loader, Validation)
    endif ()
endfunction()

function(findSDL3)
    find_package(SDL3 CONFIG)
    if (NOT SDL3_FOUND)
        find_package(Git >=2.18 REQUIRED)
        # TODO: Windows support - The git command should work, but I don't think tail or cut exist
        execute_process(COMMAND ${CMAKE_COMMAND} -E ${GIT_EXECUTABLE} -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort='version:refname' --tags https://github.com/libsdl-org/SDL.git 'release-3.*.*' | tail --lines=1 | cut --delimiter='/' --fields=3 OUTPUT_VARIABLE LATEST_RELEASE)

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

function(fetchLodepng)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.cpp ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.c)
    file(DOWNLOAD https://raw.githubusercontent.com/lvandeve/lodepng/refs/heads/master/lodepng.h ${CMAKE_BINARY_DIR}/_deps/lodepng/lodepng.h)
    set(LODEPNG_DIR "${CMAKE_BINARY_DIR}/_deps/lodepng" PARENT_SCOPE)
endfunction()