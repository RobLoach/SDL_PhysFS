set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
set(SDL_DISABLE_UNINSTALL TRUE)
set(SDL_VIDEO OFF)
set(SDL_AUDIO OFF)
set(SDL_GPU OFF)
set(SDL_RENDER OFF)
set(SDL_UNIX_CONSOLE_BUILD ON)

include(FetchContent)
# OVERRIDE_FIND_PACKAGE makes later find_package(SDL3 CONFIG) calls, like the
# one in PhysFS's CMakeLists.txt, use this fetched SDL3 (CMake 3.24+).
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.18
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(SDL3)
