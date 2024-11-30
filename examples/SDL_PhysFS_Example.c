#include <SDL3/SDL.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

int main() {
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    const int screenWidth = 800;
    const int screenHeight = 450;
	SDL_Window* window;// = SDL_CreateWindow("SDL_PhysFS: Example", 800, 450, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer;// = SDL_CreateRenderer(window, NULL);

    SDL_CreateWindowAndRenderer("SDL_PhysFS: Example", 800, 450, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Initialize PhysFS.
    SDL_PhysFS_Init(NULL);

    // Mount
    SDL_PhysFS_Mount("resources", "res");

    // Load the BMP from the SDL_RWops.
    SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, bmp);
    const int textureWidth = bmp->w;
    const int textureHeight = bmp->h;
	SDL_DestroySurface(bmp);

    // Load WAV
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
    SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength);
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wavSpec, NULL, NULL);

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
    SDL_CloseAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

    SDL_PhysFS_Quit();
	SDL_Quit();

    return 0;
}
