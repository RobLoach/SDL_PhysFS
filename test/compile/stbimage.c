/**
 * Expands SDL_PhysFS_STBIMG_Load() against the real SDL_stbimage.h, so that
 * any drift from STBIMG_Load_IO() fails to compile.
 */
#include <SDL3/SDL.h>
#include "SDL_stbimage.h"
#include "SDL_PhysFS.h"

SDL_Surface* SDL_PhysFS_Test_STBIMG_Load(const char* filename);

SDL_Surface* SDL_PhysFS_Test_STBIMG_Load(const char* filename) {
    return SDL_PhysFS_STBIMG_Load(filename);
}
