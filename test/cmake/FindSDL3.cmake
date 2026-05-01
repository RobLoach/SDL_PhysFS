set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
set(SDL_DISABLE_UNINSTALL TRUE)
set(SDL_VIDEO OFF)
set(SDL_AUDIO OFF)
set(SDL_GPU OFF)
set(SDL_RENDER OFF)
set(SDL_UNIX_CONSOLE_BUILD ON)

include(FetchContent)
FetchContent_Declare(
    SDL3Source
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.4
)
FetchContent_MakeAvailable(SDL3Source)
