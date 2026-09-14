/**
 * Includes the header without SDL_PHYSFS_IMPLEMENTATION, so every function it
 * calls has to resolve against the copy compiled into implementation.c.
 *
 * physfs.h is included first so that SDL_PhysFS_OpenIO() is declared too.
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

    if (!SDL_PhysFS_Quit()) {
        return 1;
    }

    return 0;
}
