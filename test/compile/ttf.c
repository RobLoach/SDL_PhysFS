/**
 * Expands SDL_PhysFS_TTF_OpenFont() against the real SDL3_ttf header, so that
 * any drift from TTF_OpenFontIO() fails to compile.
 */
#include <SDL3_ttf/SDL_ttf.h>
#include "SDL_PhysFS.h"

TTF_Font* SDL_PhysFS_Test_TTF_OpenFont(const char* filename, float ptsize);

TTF_Font* SDL_PhysFS_Test_TTF_OpenFont(const char* filename, float ptsize) {
    return SDL_PhysFS_TTF_OpenFont(filename, ptsize);
}
