cmake_minimum_required(VERSION 3.11)
project(SDL_physfs
    DESCRIPTION "SDL_physfs: Load SDL assets through physfs."
    HOMEPAGE_URL "https://github.com/robloach/SDL_physfs"
    VERSION 1.0.0
    LANGUAGES C
)

# Options
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SDL_PHYSFS_IS_MAIN TRUE)
else()
    set(SDL_PHYSFS_IS_MAIN FALSE)
endif()
option(SDL_PHYSFS_BUILD_EXAMPLES "Examples" ${SDL_PHYSFS_IS_MAIN})

# SDL_PhysFS
add_library(SDL_PhysFS INTERFACE)
target_include_directories(SDL_PhysFS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(FILES
    include/SDL_PhysFS.h
    DESTINATION include
)

# Examples
if (SDL_PHYSFS_BUILD_EXAMPLES)
    add_subdirectory(examples)

    # Testing
    include(CTest)
    enable_testing()
    if (BUILD_TESTING)
        # set(CTEST_CUSTOM_TESTS_IGNORE
        #     pkg-config--static
        # )
        add_subdirectory(test)
    endif()
endif()
