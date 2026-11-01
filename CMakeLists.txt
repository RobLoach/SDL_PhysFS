cmake_minimum_required(VERSION 3.16)
project(SDL_PhysFS
    DESCRIPTION "SDL_PhysFS: Load SDL assets through PhysFS."
    HOMEPAGE_URL "https://github.com/RobLoach/SDL_PhysFS"
    VERSION 3.3.0
    LANGUAGES C
)

# By default, only install when SDL_PhysFS is the top-level project, so that
# projects pulling it in with add_subdirectory() or FetchContent do not get it
# installed into their prefix.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(SDL_PHYSFS_IS_TOP_LEVEL ON)
else()
    set(SDL_PHYSFS_IS_TOP_LEVEL OFF)
endif()
option(SDL_PHYSFS_INSTALL "Install SDL_PhysFS" ${SDL_PHYSFS_IS_TOP_LEVEL})

# SDL_PhysFS
include(GNUInstallDirs)
add_library(SDL_PhysFS INTERFACE)
add_library(SDL_PhysFS::SDL_PhysFS ALIAS SDL_PhysFS)
target_include_directories(SDL_PhysFS INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install
if (SDL_PHYSFS_INSTALL)
    include(CMakePackageConfigHelpers)
    set(SDL_PHYSFS_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SDL_PhysFS")
    install(TARGETS SDL_PhysFS EXPORT SDL_PhysFSTargets)
    install(FILES include/SDL_PhysFS.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(EXPORT SDL_PhysFSTargets
        FILE SDL_PhysFSTargets.cmake
        NAMESPACE SDL_PhysFS::
        DESTINATION "${SDL_PHYSFS_CMAKE_DIR}"
    )
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL_PhysFSConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfig.cmake"
        INSTALL_DESTINATION "${SDL_PHYSFS_CMAKE_DIR}"
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SDL_PhysFSConfigVersion.cmake"
        DESTINATION "${SDL_PHYSFS_CMAKE_DIR}"
    )
endif()

# Testing
if (SDL_PHYSFS_IS_TOP_LEVEL)
    include(CTest)
    enable_testing()
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
