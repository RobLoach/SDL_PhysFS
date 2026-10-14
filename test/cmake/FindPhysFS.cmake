# physfs

# You can change these to support other formats
set(PHYSFS_ARCHIVE_ZIP ON CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_7Z OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_GRP OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_WAD OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_HOG OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_MVL OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_QPAK OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_SLB OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_ISO9660 OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_VDF OFF CACHE BOOL "" FORCE)

# Platform: Force use of the PhysFS SDL3 Platform
set(PHYSFS_PLATFORM_SDL3 ON CACHE BOOL "" FORCE)

# library options
set(PHYSFS_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PHYSFS_INSTALL OFF CACHE BOOL "" FORCE)
# Pinned to a commit rather than a tag so that builds stay reproducible.
# PHYSFS_PLATFORM_SDL3 above is only available on main, as the newest PhysFS
# release (3.2.0) predates it, so there is no tag to track instead.
include(FetchContent)
FetchContent_Declare(
    physfs
    GIT_REPOSITORY https://github.com/icculus/physfs.git
    GIT_TAG c30aa9086d4caf7548ee76655376aa7aa9098ffc
)
FetchContent_MakeAvailable(physfs)
include_directories(${physfs_SOURCE_DIR}/src)
