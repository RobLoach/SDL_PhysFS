#include <SDL3/SDL.h>
#include <string.h>

#define SDL_PHYSFS_IMPLEMENTATION
#include "SDL_PhysFS.h"

static SDL_EnumerationResult SDLCALL enumerateCounter(void* userdata, const char* dirname, const char* fname) {
    (void)dirname;
    (void)fname;
    (*(int*)userdata)++;
    return SDL_ENUM_CONTINUE;
}

// Looks for a name in a NULL-terminated list of filenames.
static bool listContains(char** files, const char* name) {
    for (char** file = files; *file != NULL; file++) {
        if (SDL_strcmp(*file, name) == 0) {
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[]) {
    (void)argc;

    // SDL_Init
    SDL_assert(SDL_Init(0));

    // SDL_PhysFS_Init / SDL_PhysFS_Quit cycle
    SDL_assert(SDL_PhysFS_Init(argv[0]));
    SDL_assert(SDL_PhysFS_Quit());

    // SDL_PhysFS_InitEx
    SDL_assert(SDL_PhysFS_InitEx(argv[0], "SDL_PhysFS", "Test"));

    // SDL_PhysFS_SetWriteDir / SDL_PhysFS_GetWriteDir
    {
        char* prefPath = SDL_GetPrefPath("SDL_PhysFS", "Test");
        SDL_assert(prefPath != NULL);
        SDL_assert(SDL_PhysFS_SetWriteDir(prefPath));
        SDL_assert(SDL_PhysFS_GetWriteDir() != NULL);
        SDL_assert(SDL_strlen(SDL_PhysFS_GetWriteDir()) > 0);
        SDL_free(prefPath);
    }

    // SDL_PhysFS_Mount
    SDL_assert(SDL_PhysFS_Mount("resources", "res"));

    // SDL_PhysFS_LoadBMP
    {
        SDL_Surface* bmp = SDL_PhysFS_LoadBMP("res/test.bmp");
        SDL_assert(bmp != NULL);
        SDL_assert(bmp->w == 250);
        SDL_assert(bmp->h == 239);
        SDL_DestroySurface(bmp);
    }

    // SDL_PhysFS_LoadFile
    {
        size_t size;
        const char* text = (const char*)SDL_PhysFS_LoadFile("res/test.txt", &size);
        SDL_assert(text != NULL);
        SDL_assert(size >= 12);
        SDL_assert(memcmp(text, "Hello, World", 12) == 0);
        SDL_free((void*)text);
    }

    // SDL_PhysFS_LoadFile with NULL datasize
    {
        void* data = SDL_PhysFS_LoadFile("res/test.txt", NULL);
        SDL_assert(data != NULL);
        SDL_free(data);
    }

    // SDL_PhysFS_LoadWAV
    {
        SDL_AudioSpec wavSpec;
        Uint32 wavLength;
        Uint8* wavBuffer;
        SDL_assert(SDL_PhysFS_LoadWAV("res/test.wav", &wavSpec, &wavBuffer, &wavLength));
        SDL_assert(wavBuffer != NULL);
        SDL_assert(wavLength > 200);
        SDL_assert(wavSpec.channels >= 2);
        SDL_free(wavBuffer);
    }

    // SDL_PhysFS_IOFromFile
    {
        SDL_IOStream* io = SDL_PhysFS_IOFromFile("res/test.txt");
        SDL_assert(io != NULL);
        SDL_assert(SDL_GetIOSize(io) > 0);
        SDL_CloseIO(io);
    }

    // Reading to the end of an IOStream reports SDL_IO_STATUS_EOF
    {
        SDL_IOStream* io = SDL_PhysFS_IOFromFile("res/test.txt");
        SDL_assert(io != NULL);
        char buffer[256];
        while (SDL_ReadIO(io, buffer, sizeof(buffer)) > 0) {
            // Keep reading until the end of the file.
        }
        SDL_assert(SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF);
        SDL_CloseIO(io);
    }

    // Seeking an IOStream, which exercises the SDL_PhysFS_SeekIO() callback
    // through every SDL_IOWhence. res/test.txt is the 13 bytes "Hello, World!".
    {
        SDL_IOStream* io = SDL_PhysFS_IOFromFile("res/test.txt");
        SDL_assert(io != NULL);
        SDL_assert(SDL_GetIOSize(io) == 13);

        char buffer[16];

        // A fresh stream sits at the start of the file.
        SDL_assert(SDL_TellIO(io) == 0);

        // SDL_IO_SEEK_SET seeks from the start.
        SDL_assert(SDL_SeekIO(io, 7, SDL_IO_SEEK_SET) == 7);
        SDL_assert(SDL_ReadIO(io, buffer, 5) == 5);
        SDL_assert(memcmp(buffer, "World", 5) == 0);

        // SDL_IO_SEEK_CUR with a zero offset reports the position without
        // moving it, which is the path SDL_TellIO() takes.
        SDL_assert(SDL_SeekIO(io, 0, SDL_IO_SEEK_CUR) == 12);
        SDL_assert(SDL_SeekIO(io, 0, SDL_IO_SEEK_CUR) == 12);

        // SDL_IO_SEEK_CUR seeks backwards and forwards from there.
        SDL_assert(SDL_SeekIO(io, -5, SDL_IO_SEEK_CUR) == 7);
        SDL_assert(SDL_ReadIO(io, buffer, 1) == 1);
        SDL_assert(buffer[0] == 'W');
        SDL_assert(SDL_SeekIO(io, 4, SDL_IO_SEEK_CUR) == 12);
        SDL_assert(SDL_ReadIO(io, buffer, 1) == 1);
        SDL_assert(buffer[0] == '!');

        // SDL_IO_SEEK_END seeks from the end of the file.
        SDL_assert(SDL_SeekIO(io, 0, SDL_IO_SEEK_END) == 13);
        SDL_assert(SDL_SeekIO(io, -6, SDL_IO_SEEK_END) == 7);
        SDL_assert(SDL_ReadIO(io, buffer, 5) == 5);
        SDL_assert(memcmp(buffer, "World", 5) == 0);

        // Seeking back to the start still works after reaching the end.
        SDL_assert(SDL_SeekIO(io, 0, SDL_IO_SEEK_END) == 13);
        SDL_assert(SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) == 0);
        SDL_assert(SDL_ReadIO(io, buffer, 5) == 5);
        SDL_assert(memcmp(buffer, "Hello", 5) == 0);

        // Seeking before the start of the file fails, whichever whence asks
        // for it, and leaves the position where it was.
        SDL_assert(SDL_SeekIO(io, -1, SDL_IO_SEEK_SET) == -1);
        SDL_assert(SDL_SeekIO(io, -100, SDL_IO_SEEK_CUR) == -1);
        SDL_assert(SDL_SeekIO(io, -100, SDL_IO_SEEK_END) == -1);
        SDL_assert(SDL_TellIO(io) == 5);

        SDL_CloseIO(io);
    }

    // SDL_PhysFS_WriteFile and read-back
    SDL_assert(SDL_PhysFS_WriteFile("test.txt", "Hello World!", 12) == 12);
    {
        size_t datasize;
        const char* data = (const char*)SDL_PhysFS_LoadFile("pref/test.txt", &datasize);
        SDL_assert(data != NULL);
        SDL_assert(datasize == 12);
        SDL_assert(memcmp(data, "Hello World!", 12) == 0);
        SDL_free((void*)data);
    }

    // SDL_PhysFS_MountFromMemory
    {
        size_t zipSize;
        void* zipData = SDL_PhysFS_LoadFile("res/test.zip", &zipSize);
        SDL_assert(zipData != NULL);
        SDL_assert(SDL_PhysFS_MountFromMemory((const unsigned char*)zipData, zipSize, "test.zip", "zip"));
        SDL_assert(SDL_PhysFS_Exists("zip/test.txt"));
        {
            size_t size;
            const char* text = (const char*)SDL_PhysFS_LoadFile("zip/test.txt", &size);
            SDL_assert(text != NULL);
            SDL_assert(memcmp(text, "Hello, World", 12) == 0);
            SDL_free((void*)text);
        }
        SDL_assert(SDL_PhysFS_Unmount("test.zip"));
        SDL_free(zipData);
    }

    // SDL_PhysFS_MountFromIO
    {
        SDL_IOStream* io = SDL_PhysFS_IOFromFile("res/test.zip");
        SDL_assert(io != NULL);
        SDL_assert(SDL_PhysFS_MountFromIO(io, "testio.zip", "zipio", true));
        SDL_assert(SDL_PhysFS_Exists("zipio/test.txt"));
        {
            size_t size;
            const char* text = (const char*)SDL_PhysFS_LoadFile("zipio/test.txt", &size);
            SDL_assert(text != NULL);
            SDL_assert(memcmp(text, "Hello, World", 12) == 0);
            SDL_free((void*)text);
        }
        SDL_assert(SDL_PhysFS_Unmount("testio.zip"));
    }

    // SDL_PhysFS_CreateDirectory / SDL_PhysFS_RemovePath
    {
        // SDL_PhysFS_WriteFile() cannot create missing parent directories, so
        // the directory has to exist before writing into it.
        SDL_assert(SDL_PhysFS_CreateDirectory("saves"));

        // Creating a directory that already exists is not an error.
        SDL_assert(SDL_PhysFS_CreateDirectory("saves"));

        SDL_assert(SDL_PhysFS_WriteFile("saves/slot1.sav", "save", 4) == 4);
        SDL_assert(SDL_PhysFS_Exists("pref/saves/slot1.sav") == true);

        // Paths here are relative to the write directory, whereas
        // SDL_PhysFS_Exists() searches the mounted tree, where the write
        // directory sits at pref.
        SDL_assert(SDL_PhysFS_RemovePath("saves/slot1.sav"));
        SDL_assert(SDL_PhysFS_Exists("pref/saves/slot1.sav") == false);

        // The directory is empty now, so it can be removed too.
        SDL_assert(SDL_PhysFS_RemovePath("saves"));
        SDL_assert(SDL_PhysFS_Exists("pref/saves") == false);

        // Removing something that is not there fails.
        SDL_assert(SDL_PhysFS_RemovePath("saves/slot1.sav") == false);

        // NULL paths are rejected rather than passed through.
        SDL_assert(SDL_PhysFS_CreateDirectory(NULL) == false);
        SDL_assert(SDL_PhysFS_RemovePath(NULL) == false);
    }

    // SDL_PhysFS_Exists
    SDL_assert(SDL_PhysFS_Exists("res/test.bmp") == true);
    SDL_assert(SDL_PhysFS_Exists("res/notfound.txt") == false);

    // SDL_PhysFS_GetPathInfo
    {
        SDL_PathInfo info;

        // A regular file reports its type, size and modification time.
        SDL_assert(SDL_PhysFS_GetPathInfo("res/test.txt", &info));
        SDL_assert(info.type == SDL_PATHTYPE_FILE);
        SDL_assert(info.size == 13u);
        SDL_assert(info.modify_time > 0);

        // A directory reports no size. Unlike SDL_PhysFS_Exists(), which is
        // true for both of these, the type tells the two apart.
        SDL_assert(SDL_PhysFS_GetPathInfo("res", &info));
        SDL_assert(info.type == SDL_PATHTYPE_DIRECTORY);
        SDL_assert(info.size == 0u);
        SDL_assert(SDL_PhysFS_Exists("res") == true);

        // Files inside an archive are reported the same way.
        SDL_assert(SDL_PhysFS_GetPathInfo("res/test.zip", &info));
        SDL_assert(info.type == SDL_PATHTYPE_FILE);
        SDL_assert(info.size > 0u);

        // A NULL info only tests for existence.
        SDL_assert(SDL_PhysFS_GetPathInfo("res/test.txt", NULL) == true);
        SDL_assert(SDL_PhysFS_GetPathInfo("res/notfound.txt", NULL) == false);

        // Missing paths fail, and clear the struct rather than leaving it as it
        // was found.
        SDL_assert(SDL_PhysFS_GetPathInfo("res/notfound.txt", &info) == false);
        SDL_assert(info.type == SDL_PATHTYPE_NONE);
        SDL_assert(info.size == 0u);
        SDL_assert(info.modify_time == 0);

        // NULL paths are rejected rather than passed through.
        SDL_assert(SDL_PhysFS_GetPathInfo(NULL, &info) == false);
        SDL_assert(SDL_PhysFS_GetPathInfo(NULL, NULL) == false);
    }

    // SDL_PhysFS_GetVersion
    SDL_assert(SDL_PhysFS_GetVersion() > 2);

    // SDL_PhysFS_LoadDirectoryFiles
    int resourceCount = 0;
    {
        char** files = SDL_PhysFS_LoadDirectoryFiles("res");
        SDL_assert(files != NULL);

        for (char** file = files; *file != NULL; file++) {
            resourceCount++;
        }

        // Assert on the resources the tests actually depend on, rather than on
        // a hard-coded total that breaks whenever a file is added to
        // test/resources/.
        SDL_assert(listContains(files, "test.bmp"));
        SDL_assert(listContains(files, "test.txt"));
        SDL_assert(listContains(files, "test.wav"));
        SDL_assert(listContains(files, "test.zip"));
        SDL_assert(listContains(files, "notfound.txt") == false);
        SDL_assert(resourceCount >= 4);

        SDL_PhysFS_FreeDirectoryFiles(files);
    }

    // SDL_PhysFS_EnumerateDirectory walks the same entries.
    {
        int count = 0;
        SDL_assert(SDL_PhysFS_EnumerateDirectory("res", enumerateCounter, &count));
        SDL_assert(count == resourceCount);
    }

    // SDL_PhysFS_IOStatus
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_OK) == SDL_IO_STATUS_READY);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_PAST_EOF) == SDL_IO_STATUS_EOF);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_OUT_OF_MEMORY) == SDL_IO_STATUS_ERROR);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_UNSUPPORTED) == SDL_IO_STATUS_ERROR);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_NOT_INITIALIZED) == SDL_IO_STATUS_NOT_READY);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_IO) == SDL_IO_STATUS_ERROR);

    // SDL_PhysFS_Unmount
    SDL_assert(SDL_PhysFS_Unmount("resources") == true);

    // SDL_PhysFS_Quit
    SDL_assert(SDL_PhysFS_Quit() == true);

    SDL_Quit();

    return 0;
}
