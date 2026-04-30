# SDL_PhysFS

[PhysicsFS](https://github.com/icculus/physfs) is a portable flexible file I/O abstraction library. `SDL_PhysFS` builds upon [PhysFS's physfsrwops.h](https://github.com/icculus/physfs/blob/main/extras/physfsrwops.h) to ease the integration between SDL and PhysFS to load assets from *.zip* files.

## Dependencies

- [SDL](https://github.com/libsdl-org/SDL) 3.x
  - For SDL 2, see the [2.x branch](https://github.com/RobLoach/SDL_PhysFS/tree/2.x)
- [PhysFS](https://github.com/icculus/physfs)

## Example

``` c
#include <SDL3/SDL.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    // Initialize PhysFS
    SDL_PhysFS_Init(argv[0]);

    // Mount
    SDL_PhysFS_Mount("assets.zip", "assets");

    // Load a surface from assets.zip
    SDL_Surface* dog = SDL_PhysFS_LoadBMP("assets/dog.bmp");

    // Clean up
    SDL_PhysFS_Quit();
    SDL_Quit();

    return 0;
}
```

### API

``` c
bool SDL_PhysFS_Init(const char* argv);
bool SDL_PhysFS_InitEx(const char* argv, const char* org, const char* app);
bool SDL_PhysFS_Quit();
bool SDL_PhysFS_Mount(const char* newDir, const char* mountPoint);
bool SDL_PhysFS_MountFromMemory(const unsigned char *fileData, int dataSize, const char* newDir, const char* mountPoint);
bool SDL_PhysFS_Unmount(const char* oldDir);
SDL_IOStream* SDL_PhysFS_IOFromFile(const char* filename);
SDL_Surface* SDL_PhysFS_LoadBMP(const char* filename);
bool SDL_PhysFS_LoadWAV(const char* filename, SDL_AudioSpec* spec, Uint8** audio_buf, Uint32* audio_len);
void* SDL_PhysFS_LoadFile(const char* filename, size_t* datasize);
size_t SDL_PhysFS_WriteFile(const char* file, const void* buffer, size_t size);
bool SDL_PhysFS_SetWriteDir(const char* path);
char** SDL_PhysFS_LoadDirectoryFiles(const char* directory);
void SDL_PhysFS_FreeDirectoryFiles(char** files);
bool SDL_PhysFS_Exists(const char* file);

// Optional Integrations
SDL_Surface* SDL_PhysFS_IMG_Load(const char* filename);    // SDL_image
Mix_Music* SDL_PhysFS_Mix_LoadMUS(const char* filename);   // SDL_mixer
TTF_Font* SDL_PhysFS_TTF_OpenFont(const char* filename, int ptsize); // SDL_ttf
SDL_Surface* SDL_PhysFS_STBIMG_Load(const char* filename); // SDL_stbimage.h
```

## License

[zlib](LICENSE)
