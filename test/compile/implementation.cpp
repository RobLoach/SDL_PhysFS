/**
 * The C++ half of the link check: builds the implementation as C++ to make sure
 * the header stays valid C++ and that extern "C" covers every definition.
 */
#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"
