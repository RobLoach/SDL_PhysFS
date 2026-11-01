/**
 * Includes the header from C++ without SDL_PHYSFS_IMPLEMENTATION, and links
 * against implementation.cpp.
 */
#include "physfs.h"
#include "SDL_PhysFS.h"

int main(int argc, char* argv[]) {
    (void)argc;

    if (SDL_PhysFS_GetVersion() < SDL_VERSIONNUM(3, 0, 0)) {
        return 1;
    }

    if (!SDL_PhysFS_Init(argv[0])) {
        return 1;
    }

    if (SDL_PhysFS_Exists("nothing-is-mounted-here.txt")) {
        return 1;
    }

    if (SDL_PhysFS_GetWriteDir() != NULL) {
        return 1;
    }

    SDL_PathInfo info;
    if (SDL_PhysFS_GetPathInfo("nothing-is-mounted-here.txt", &info)) {
        return 1;
    }

    // Nothing is mounted and there is no write directory, so neither of these
    // can succeed. They are called to make the linker resolve them.
    if (SDL_PhysFS_CreateDirectory("nothing-is-mounted-here")) {
        return 1;
    }

    if (SDL_PhysFS_RemovePath("nothing-is-mounted-here")) {
        return 1;
    }

    if (!SDL_PhysFS_Quit()) {
        return 1;
    }

    return 0;
}
