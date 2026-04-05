set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
set(SDL_DISABLE_UNINSTALL TRUE)

include(FetchContent)
FetchContent_Declare(
    SDL3Source
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.4
)
FetchContent_MakeAvailable(SDL3Source)
