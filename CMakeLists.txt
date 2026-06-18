cmake_minimum_required(VERSION 3.11)
project(SDL_PhysFS
    DESCRIPTION "SDL_PhysFS: Load SDL assets through PhysFS."
    HOMEPAGE_URL "https://github.com/RobLoach/SDL_PhysFS"
    VERSION 3.1.0
    LANGUAGES C
)

# SDL_PhysFS
add_library(SDL_PhysFS INTERFACE)
target_include_directories(SDL_PhysFS INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Install
include(CMakePackageConfigHelpers)
install(TARGETS SDL_PhysFS EXPORT SDL_PhysFSTargets)
install(FILES include/SDL_PhysFS.h DESTINATION include)
install(EXPORT SDL_PhysFSTargets
    FILE SDL_PhysFSConfig.cmake
    NAMESPACE SDL_PhysFS::
    DESTINATION lib/cmake/SDL_PhysFS
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfigVersion.cmake"
    DESTINATION lib/cmake/SDL_PhysFS
)

# Testing
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    include(CTest)
    enable_testing()
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
