cmake_minimum_required(VERSION 3.11)
project(SDL_PhysFS
    DESCRIPTION "SDL_PhysFS: Load SDL assets through PhysFS."
    HOMEPAGE_URL "https://github.com/RobLoach/SDL_PhysFS"
    VERSION 3.1.0
    LANGUAGES C
)

# SDL_PhysFS
add_library(SDL_PhysFS INTERFACE)
target_include_directories(SDL_PhysFS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
install(FILES
    include/SDL_PhysFS.h
    DESTINATION include
)

# Testing
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    include(CTest)
    enable_testing()
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
