/**
 * Expands SDL_PhysFS_MIX_LoadAudio() against the real SDL3_mixer header, so
 * that any drift from MIX_LoadAudio_IO() fails to compile.
 */
#include <SDL3_mixer/SDL_mixer.h>
#include "SDL_PhysFS.h"

MIX_Audio* SDL_PhysFS_Test_MIX_LoadAudio(MIX_Mixer* mixer, const char* filename, bool predecode);

MIX_Audio* SDL_PhysFS_Test_MIX_LoadAudio(MIX_Mixer* mixer, const char* filename, bool predecode) {
    return SDL_PhysFS_MIX_LoadAudio(mixer, filename, predecode);
}
