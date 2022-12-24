#include <SDL2/SDL.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_physfs.h"

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);

    const int screenWidth = 800;
    const int screenHeight = 450;
	SDL_Window* window = SDL_CreateWindow("SDL_physfs: Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 450, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Initialize PhysFS.
    SDL_PhysFS_Init();

    // Mount
    SDL_PhysFS_Mount("resources", "res");

    // Load the BMP from the SDL_RWops.
    SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, bmp);
    const int textureWidth = bmp->w;
    const int textureHeight = bmp->h;
	SDL_FreeSurface(bmp);

    // Load WAV
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
    SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength);
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    // Play the wav
    SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);

    int quit = 0;
    SDL_Event event;

    while (quit == 0) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = 1;
                        break;
                        case SDLK_SPACE:
                            SDL_QueueAudio(deviceId, wavBuffer, wavLength);
                            SDL_PauseAudioDevice(deviceId, 0);
                        break;
                    }
                    break;
            }
        }

        SDL_Rect destination = {
            screenWidth / 2 - textureWidth / 2,
            screenHeight / 2 - textureHeight / 2,
            textureWidth,
            textureHeight
        };

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, &destination);
		SDL_RenderPresent(renderer);
    }

    SDL_FreeWAV(wavBuffer);
    SDL_CloseAudioDevice(deviceId);

    SDL_PhysFS_Quit();
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}