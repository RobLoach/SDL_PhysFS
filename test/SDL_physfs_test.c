#include <SDL2/SDL.h>
#include <string.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_physfs.h"

int main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // SDL_PhysFS_Init
    SDL_PhysFS_Init();

    // SDL_PhysFS_Mount
    SDL_PhysFS_Mount("resources", "res");

    // SDL_PhysFS_LoadBMP
    {
        SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
        SDL_assert(bmp->w == 250);
        SDL_assert(bmp->h == 239);
        SDL_FreeSurface(bmp);
    }

    // SDL_PhysFS_LoadFile
    {
        size_t size;
        const char* text = (const char*)SDL_PhysFS_LoadFile("res/test.txt", &size);
        SDL_assert(strcmp(text, "Hello, World!") == 0);
        SDL_free((void*)text);
    }

    // SDL_PhysFS_LoadWAV
    {
        SDL_AudioSpec wavSpec;
        Uint32 wavLength;
        Uint8 *wavBuffer;
        SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength);
        SDL_assert(wavLength > 200);
        SDL_assert(wavSpec.channels >= 2);
        SDL_FreeWAV(wavBuffer);
    }

    // SDL_PhysFS_Unmount
    SDL_PhysFS_Unmount("resources");

    SDL_PhysFS_Quit();
	SDL_Quit();
    return 0;
}
