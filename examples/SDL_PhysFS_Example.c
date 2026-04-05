#include <SDL3/SDL.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

int main() {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    const int screenWidth = 800;
    const int screenHeight = 450;
    SDL_Window* window;
    SDL_Renderer* renderer;

    if (!SDL_CreateWindowAndRenderer("SDL_PhysFS: Example", 800, 450, 0, &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Initialize PhysFS.
    if (!SDL_PhysFS_Init(NULL)) {
        SDL_Log("SDL_PhysFS_Init failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Mount
    SDL_PhysFS_Mount("resources", "res");

    // Load the BMP from the SDL_IOStream.
    SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
    if (bmp == NULL) {
        SDL_Log("SDL_PhysFS_LoadBMP failed: %s", SDL_GetError());
        SDL_PhysFS_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, bmp);
    if (tex == NULL) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_DestroySurface(bmp);
        SDL_PhysFS_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    const int textureWidth = bmp->w;
    const int textureHeight = bmp->h;
    SDL_DestroySurface(bmp);

    // Load WAV
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
    if (!SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength)) {
        SDL_Log("SDL_PhysFS_LoadWAV failed: %s", SDL_GetError());
        SDL_DestroyTexture(tex);
        SDL_PhysFS_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wavSpec, NULL, NULL);
    if (stream == NULL) {
        SDL_Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        SDL_free(wavBuffer);
        SDL_DestroyTexture(tex);
        SDL_PhysFS_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Play the wav
    SDL_ResumeAudioStreamDevice(stream);

    int quit = 0;
    SDL_Event event;

    while (quit == 0) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = 1;
                    break;
                case SDL_EVENT_KEY_UP:
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                    }
                    break;
            }
        }

        if (SDL_GetAudioStreamAvailable(stream) < (int)wavLength) {
            /* feed more data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
            SDL_PutAudioStreamData(stream, wavBuffer, wavLength);
        }

        SDL_FRect destination = {
            screenWidth / 2 - textureWidth / 2,
            screenHeight / 2 - textureHeight / 2,
            textureWidth,
            textureHeight
        };

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, tex, NULL, &destination);
        SDL_RenderPresent(renderer);
    }

    SDL_free(wavBuffer);
    SDL_DestroyAudioStream(stream);

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_PhysFS_Quit();
    SDL_Quit();

    return 0;
}
