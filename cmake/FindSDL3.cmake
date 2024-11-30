set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
set(SDL_DISABLE_UNINSTALL TRUE)

include(FetchContent)
FetchContent_Declare(
    SDL3Source
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 3a1d76d298db023f6cf37fb08ee766f20a4e12ab
)
FetchContent_MakeAvailable(SDL3Source)
