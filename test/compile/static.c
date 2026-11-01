/**
 * Builds the header in SDL_PHYSFS_STATIC mode, where every declaration and
 * definition has to agree on internal linkage.
 */
#define SDL_PHYSFS_STATIC
#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

int main(int argc, char* argv[]) {
    (void)argc;

    if (SDL_PhysFS_GetVersion() < SDL_VERSIONNUM(3, 0, 0)) {
        return 1;
    }

    if (!SDL_PhysFS_Init(argv[0])) {
        return 1;
    }

    if (!SDL_PhysFS_Quit()) {
        return 1;
    }

    return 0;
}
