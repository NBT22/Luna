set(SYSTEM_DIRECTORY_FLAG $<IF:$<OR:$<COMPILE_LANG_AND_ID:C,MSVC>,$<COMPILE_LANG_AND_ID:CXX,MSVC>>,/external:I,-isystem>)

macro(enableOptions)
    foreach (optionToSet IN ITEMS ${ARGN})
        set(${optionToSet} ON)
    endforeach ()
endmacro()

macro(disableOptions)
    foreach (optionToSet IN ITEMS ${ARGN})
        set(${optionToSet} OFF)
    endforeach ()
endmacro()

macro(ensureVersionsMatch packageOne packageOneVersion packageTwo packageTwoVersion)
    if (NOT (${packageOneVersion} STREQUAL ${packageTwoVersion}))
        message(AUTHOR_WARNING "${packageOne} version ${${packageOneVersion}} does not match ${packageTwo} version ${${packageTwoVersion}}!")
    endif ()
endmacro()

macro(getLatestPackageVersion gitRepo versionSplat)
    find_package(Git 2.18 REQUIRED)
    if (WIN32)
        execute_process(COMMAND powershell -command "((& '${GIT_EXECUTABLE}' -c 'versionsort.suffix=-' ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} '${versionSplat}' | Select-Object -Last 1) -Split '/')[2]" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE _LUNA_PACKAGE_LATEST_RELEASE_VERSION)
    else ()
        execute_process(COMMAND ${GIT_EXECUTABLE} -c "versionsort.suffix=-" ls-remote --exit-code --refs --sort=version:refname --tags ${gitRepo} "${versionSplat}" COMMAND tail --lines=1 COMMAND cut --delimiter=/ --fields=3 COMMAND tr -d "\n" OUTPUT_VARIABLE _LUNA_PACKAGE_LATEST_RELEASE_VERSION)
    endif ()
endmacro()

macro(makePackageAvailable gitRepo versionSplat packageName)
    getLatestPackageVersion(${gitRepo} ${versionSplat})

    FetchContent_Declare(
            ${packageName}
            GIT_REPOSITORY ${gitRepo}
            GIT_TAG ${_LUNA_PACKAGE_LATEST_RELEASE_VERSION}
            GIT_CONFIG submodule.active=none
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            EXCLUDE_FROM_ALL
            SYSTEM
            FIND_PACKAGE_ARGS ${ARGN}
    )
    FetchContent_MakeAvailable(${packageName})
endmacro()