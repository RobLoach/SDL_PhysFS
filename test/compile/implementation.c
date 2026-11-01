/**
 * Compiles the implementation on its own, with no main().
 *
 * Linked against header_only.c to make sure every declaration in the header
 * matches its definition, and that nothing in the implementation has picked up
 * internal linkage by accident.
 */
#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"
