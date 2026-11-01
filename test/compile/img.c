/**
 * Expands SDL_PhysFS_IMG_Load() against the real SDL3_image header, so that
 * any drift from IMG_Load_IO() fails to compile.
 */
#include <SDL3_image/SDL_image.h>
#include "SDL_PhysFS.h"

SDL_Surface* SDL_PhysFS_Test_IMG_Load(const char* filename);

SDL_Surface* SDL_PhysFS_Test_IMG_Load(const char* filename) {
    return SDL_PhysFS_IMG_Load(filename);
}
