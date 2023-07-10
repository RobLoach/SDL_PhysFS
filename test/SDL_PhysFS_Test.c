#include <SDL2/SDL.h>
#include <string.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

int main() {
    // SDL_Init
	SDL_assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0);

    // SDL_PhysFS_Init
    SDL_assert(SDL_PhysFS_InitEx("SDL_PhysFS", "Test") == SDL_TRUE);

    // SDL_PhysFS_Mount
    SDL_assert(SDL_PhysFS_Mount("resources", "res") == SDL_TRUE);

    // SDL_PhysFS_LoadBMP
    {
        SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
        SDL_assert(bmp != NULL);
        SDL_assert(bmp->w == 250);
        SDL_assert(bmp->h == 239);
        SDL_FreeSurface(bmp);
    }

    // SDL_PhysFS_LoadFile
    {
        size_t size;
        const char* text = (const char*)SDL_PhysFS_LoadFile("res/test.txt", &size);
        SDL_assert(text != NULL);
        SDL_assert(strcmp(text, "Hello, World!") == 0);
        SDL_free((void*)text);
    }

    // SDL_PhysFS_LoadWAV
    {
        SDL_AudioSpec wavSpec;
        Uint32 wavLength;
        Uint8 *wavBuffer;
        SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength);
        SDL_assert(wavBuffer != NULL);
        SDL_assert(wavLength > 200);
        SDL_assert(wavSpec.channels >= 2);
        SDL_FreeWAV(wavBuffer);
    }

    // Write
    SDL_assert(SDL_PhysFS_Write("test.txt", "Hello World!", 12) > 5);

    // Load from the preferences directory
    {
        size_t datasize;
        const char* data = (const char*)SDL_PhysFS_LoadFile("/app/test.txt", &datasize);
        SDL_assert(data != NULL);
        SDL_assert(strcmp(data, "Hello World!") == 0);
        SDL_free((void*)data);
    }

    // SDL_PhysFS_Exists
    SDL_assert(SDL_PhysFS_Exists("res/test.bmp") == SDL_TRUE);
    SDL_assert(SDL_PhysFS_Exists("res/notfound.txt") == SDL_FALSE);

    // SDL_PhysFS_LoadDirectoryFiles
    {
        char** files = SDL_PhysFS_LoadDirectoryFiles("res");
        SDL_assert(files != NULL);
        int count = 0;
        for (char** file = files; *file != NULL; file++) {
            count++;
        }
        SDL_assert(count == 3);
        SDL_PhysFS_FreeDirectoryFiles(files);
    }

    // Make sure there are no errors.
    SDL_assert(strlen(SDL_GetError()) == 0);

    // SDL_PhysFS_Unmount
    SDL_assert(SDL_PhysFS_Unmount("resources") == SDL_TRUE);

    // SDL_PhysFS_Quit
    SDL_assert(SDL_PhysFS_Quit() == SDL_TRUE);

	SDL_Quit();

    return 0;
}
