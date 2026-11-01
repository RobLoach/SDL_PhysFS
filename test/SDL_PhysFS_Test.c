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

// A calloc() that counts its calls, and fails the one numbered failCallocAt,
// used to make SDL_OpenIO() fail, as it allocates its stream with SDL_calloc().
static SDL_calloc_func originalCalloc;
static int callocCount = 0;
static int failCallocAt = 0;

static void* SDLCALL failingCalloc(size_t nmemb, size_t size) {
    if (++callocCount == failCallocAt) {
        return NULL;
    }

    return originalCalloc(nmemb, size);
}

// A PhysFS archiver for the write directory whose files fail to write or flush
// on demand, as forcing a real disk to do so is not practical. The flags can be
// cleared again afterwards, so that SDL_PhysFS_Quit() can close the files.
#define FAILING_EXTENSION "sdlphysfsfailing"
static bool failingWrite = false;
static bool failingFlush = false;

static PHYSFS_sint64 failingIoRead(PHYSFS_Io* io, void* buf, PHYSFS_uint64 len) {
    (void)io;
    (void)buf;
    (void)len;
    PHYSFS_setErrorCode(PHYSFS_ERR_OPEN_FOR_WRITING);
    return -1;
}

static PHYSFS_sint64 failingIoWrite(PHYSFS_Io* io, const void* buffer, PHYSFS_uint64 len) {
    (void)io;
    (void)buffer;
    if (failingWrite) {
        PHYSFS_setErrorCode(PHYSFS_ERR_IO);
        return -1;
    }

    return (PHYSFS_sint64)len;
}

static int failingIoSeek(PHYSFS_Io* io, PHYSFS_uint64 offset) {
    (void)io;
    (void)offset;
    return 1;
}

static PHYSFS_sint64 failingIoTell(PHYSFS_Io* io) {
    (void)io;
    return 0;
}

static PHYSFS_sint64 failingIoLength(PHYSFS_Io* io) {
    (void)io;
    return 0;
}

static PHYSFS_Io* failingIoDuplicate(PHYSFS_Io* io) {
    (void)io;
    PHYSFS_setErrorCode(PHYSFS_ERR_UNSUPPORTED);
    return NULL;
}

static int failingIoFlush(PHYSFS_Io* io) {
    (void)io;
    if (failingFlush) {
        PHYSFS_setErrorCode(PHYSFS_ERR_IO);
        return 0;
    }

    return 1;
}

static void failingIoDestroy(PHYSFS_Io* io) {
    SDL_free(io);
}

static void* failingOpenArchive(PHYSFS_Io* io, const char* name, int forWrite, int* claimed) {
    // Only claim the write directory made for this archiver, and leave
    // everything else to the real archivers.
    const char* ext = SDL_strrchr(name, '.');
    if (!forWrite || ext == NULL || SDL_strcmp(ext + 1, FAILING_EXTENSION) != 0) {
        return NULL;
    }

    *claimed = 1;
    return io;
}

static PHYSFS_EnumerateCallbackResult failingEnumerate(void* opaque, const char* dirname, PHYSFS_EnumerateCallback cb, const char* origdir, void* callbackdata) {
    (void)opaque;
    (void)dirname;
    (void)cb;
    (void)origdir;
    (void)callbackdata;
    return PHYSFS_ENUM_OK;
}

static PHYSFS_Io* failingOpenRead(void* opaque, const char* fnm) {
    (void)opaque;
    (void)fnm;
    PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
    return NULL;
}

static PHYSFS_Io* failingOpenWrite(void* opaque, const char* filename) {
    (void)opaque;
    (void)filename;
    PHYSFS_Io* io = (PHYSFS_Io*)SDL_calloc(1, sizeof(PHYSFS_Io));
    if (io == NULL) {
        PHYSFS_setErrorCode(PHYSFS_ERR_OUT_OF_MEMORY);
        return NULL;
    }

    io->read = failingIoRead;
    io->write = failingIoWrite;
    io->seek = failingIoSeek;
    io->tell = failingIoTell;
    io->length = failingIoLength;
    io->duplicate = failingIoDuplicate;
    io->flush = failingIoFlush;
    io->destroy = failingIoDestroy;
    return io;
}

static int failingRemove(void* opaque, const char* filename) {
    (void)opaque;
    (void)filename;
    PHYSFS_setErrorCode(PHYSFS_ERR_UNSUPPORTED);
    return 0;
}

static int failingStat(void* opaque, const char* fn, PHYSFS_Stat* stat) {
    (void)opaque;
    (void)fn;
    (void)stat;
    PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
    return 0;
}

static void failingCloseArchive(void* opaque) {
    // The archive owns the PhysFS_Io of the file it was opened on.
    PHYSFS_Io* io = (PHYSFS_Io*)opaque;
    io->destroy(io);
}

static const PHYSFS_Archiver failingArchiver = {
    0,
    { FAILING_EXTENSION, "Fails to write on demand", "SDL_PhysFS", "https://github.com/RobLoach/SDL_PhysFS", 0 },
    failingOpenArchive,
    failingEnumerate,
    failingOpenRead,
    failingOpenWrite,
    failingOpenWrite,
    failingRemove,
    failingRemove,
    failingStat,
    failingCloseArchive,
};

int main(int argc, char* argv[]) {
    (void)argc;

    // SDL_Init
    SDL_assert(SDL_Init(0));

    // SDL_PhysFS_Init / SDL_PhysFS_Quit cycle
    SDL_assert(SDL_PhysFS_Init(argv[0]));
    SDL_assert(SDL_PhysFS_Quit());

    // SDL_PhysFS_InitEx keeps SDL's own error when the pref path fails,
    // rather than appending an unrelated PhysFS error to it.
    SDL_ClearError();
    SDL_assert(SDL_PhysFS_InitEx(argv[0], "SDL_PhysFS", NULL) == false);
    SDL_assert(SDL_strstr(SDL_GetError(), "app") != NULL);
    SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
    SDL_assert(SDL_strstr(SDL_GetError(), "SDL_PhysFS.h") == NULL);

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

#if !SDL_VERSION_ATLEAST(3, 6, 0)
    // SDL_PhysFS_LoadJPG reports the SDL version it needs, without a PhysFS
    // error appended to it.
    SDL_assert(SDL_PhysFS_LoadJPG("res/test.bmp") == NULL);
    SDL_assert(SDL_strstr(SDL_GetError(), "requires SDL 3.6.0") != NULL);
    SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
#endif

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
        SDL_assert(SDL_strstr(SDL_GetError(), "seek past start of file") != NULL);
        SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
        SDL_assert(SDL_SeekIO(io, -100, SDL_IO_SEEK_CUR) == -1);
        SDL_assert(SDL_SeekIO(io, -100, SDL_IO_SEEK_END) == -1);
        SDL_assert(SDL_TellIO(io) == 5);

        // An invalid whence is reported as an invalid parameter.
        SDL_assert(SDL_SeekIO(io, 0, (SDL_IOWhence)42) == -1);
        SDL_assert(SDL_strstr(SDL_GetError(), "whence") != NULL);
        SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
        SDL_assert(SDL_TellIO(io) == 5);

        SDL_CloseIO(io);
    }

    // Writing to a stream opened for reading fails, and keeps the PhysFS error
    // rather than reporting the stream as ready.
    {
        PHYSFS_File* handle = PHYSFS_openRead("res/test.txt");
        SDL_assert(handle != NULL);
        SDL_IOStream* io = SDL_PhysFS_OpenIO(handle);
        SDL_assert(io != NULL);
        SDL_assert(SDL_WriteIO(io, "data", 4) == 0);
        SDL_assert(SDL_GetIOStatus(io) == SDL_IO_STATUS_READONLY);
        SDL_assert(SDL_strstr(SDL_GetError(), "Failed to write file") != NULL);
        SDL_assert(SDL_strstr(SDL_GetError(), PHYSFS_getErrorByCode(PHYSFS_ERR_OPEN_FOR_READING)) != NULL);
        SDL_CloseIO(io);
    }

    // The PhysFS allocator callbacks refuse sizes that cannot be allocated,
    // rather than truncating them to a size_t on 32-bit platforms.
    SDL_assert(SDL_PhysFS_AllocatorMalloc(SDL_MAX_UINT64) == NULL);
    SDL_assert(SDL_PhysFS_AllocatorRealloc(NULL, SDL_MAX_UINT64) == NULL);

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

    // SDL_PhysFS_WriteFile with a size of 0 writes an empty file, which
    // SDL_PhysFS_LoadFile() then reads back as empty.
    {
        SDL_assert(SDL_PhysFS_WriteFile("empty.txt", NULL, 0) == 0);
        SDL_assert(SDL_PhysFS_Exists("pref/empty.txt") == true);

        size_t datasize = 123;
        void* data = SDL_PhysFS_LoadFile("pref/empty.txt", &datasize);
        SDL_assert(data != NULL);
        SDL_assert(datasize == 0);
        SDL_free(data);
    }

    // An empty write truncates a file that already has contents.
    {
        SDL_assert(SDL_PhysFS_WriteFile("empty.txt", "data", 4) == 4);
        SDL_assert(SDL_PhysFS_WriteFile("empty.txt", NULL, 0) == 0);

        size_t datasize = 123;
        void* data = SDL_PhysFS_LoadFile("pref/empty.txt", &datasize);
        SDL_assert(data != NULL);
        SDL_assert(datasize == 0);
        SDL_free(data);

        SDL_assert(SDL_PhysFS_RemovePath("empty.txt"));
    }

    // A NULL buffer is only allowed when nothing is being written.
    SDL_assert(SDL_PhysFS_WriteFile("invalid.txt", NULL, 4) == 0);
    SDL_assert(SDL_PhysFS_Exists("pref/invalid.txt") == false);
    SDL_assert(SDL_PhysFS_WriteFile(NULL, "data", 4) == 0);

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
        SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
        SDL_assert(SDL_PhysFS_RemovePath(NULL) == false);
        SDL_assert(SDL_strstr(SDL_GetError(), "(no error)") == NULL);
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
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_NOT_INITIALIZED) == SDL_IO_STATUS_ERROR);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_OPEN_FOR_READING) == SDL_IO_STATUS_READONLY);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_READ_ONLY) == SDL_IO_STATUS_READONLY);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_OPEN_FOR_WRITING) == SDL_IO_STATUS_WRITEONLY);
    SDL_assert(SDL_PhysFS_IOStatus(-1) == SDL_IO_STATUS_ERROR);
    SDL_assert(SDL_PhysFS_IOStatus(PHYSFS_ERR_IO) == SDL_IO_STATUS_ERROR);

    // SDL_PhysFS_IOFromFile closes the PhysFS handle when the stream cannot
    // be created, rather than leaving it open.
    {
        SDL_malloc_func mallocFunc;
        SDL_realloc_func reallocFunc;
        SDL_free_func freeFunc;
        SDL_GetMemoryFunctions(&mallocFunc, &originalCalloc, &reallocFunc, &freeFunc);
        SDL_assert(SDL_SetMemoryFunctions(mallocFunc, failingCalloc, reallocFunc, freeFunc));

        // PhysFS may open the file through SDL as well, so count the calls a
        // successful open makes, and fail only the last one, which is the
        // stream SDL_PhysFS_OpenIO() creates.
        callocCount = 0;
        SDL_IOStream* io = SDL_PhysFS_IOFromFile("res/test.txt");
        SDL_assert(io != NULL);
        SDL_assert(SDL_CloseIO(io));
        SDL_assert(callocCount > 0);

        failCallocAt = callocCount;
        callocCount = 0;
        io = SDL_PhysFS_IOFromFile("res/test.txt");
        failCallocAt = 0;
        SDL_assert(SDL_SetMemoryFunctions(mallocFunc, originalCalloc, reallocFunc, freeFunc));
        SDL_assert(io == NULL);
        SDL_assert(SDL_strstr(SDL_GetError(), "Failed to open file") == NULL);

        // PhysFS refuses to unmount an archive with files still open in it, so
        // this fails if the handle was leaked.
        SDL_assert(SDL_PhysFS_Unmount("resources"));
        SDL_assert(SDL_PhysFS_Mount("resources", "res"));
    }

    // SDL_PhysFS_Unmount
    SDL_assert(SDL_PhysFS_Unmount("resources") == true);

    // SDL_PhysFS_Quit
    SDL_assert(SDL_PhysFS_Quit() == true);

    // Failures to write, flush and close files are reported, using a write
    // directory whose files fail on demand.
    {
        SDL_assert(SDL_PhysFS_Init(argv[0]));
        SDL_assert(PHYSFS_registerArchiver(&failingArchiver));

        // PhysFS only hands a path that is not a directory to an archiver, so
        // the write directory has to be a file.
        char* prefPath = SDL_GetPrefPath("SDL_PhysFS", "Test");
        SDL_assert(prefPath != NULL);
        char* writeDir = NULL;
        SDL_assert(SDL_asprintf(&writeDir, "%swritedir." FAILING_EXTENSION, prefPath) > 0);
        SDL_free(prefPath);
        SDL_IOStream* file = SDL_IOFromFile(writeDir, "wb");
        SDL_assert(file != NULL);
        SDL_assert(SDL_CloseIO(file));
        SDL_assert(SDL_PhysFS_SetWriteDir(writeDir));

        // Writing works while nothing fails.
        SDL_assert(SDL_PhysFS_WriteFile("ok.txt", "data", 4) == 4);

        // A failed write is a failed SDL_PhysFS_WriteFile().
        failingWrite = true;
        SDL_assert(SDL_PhysFS_WriteFile("write.txt", "data", 4) == 0);
        SDL_assert(SDL_strstr(SDL_GetError(), "Failed to write data to file") != NULL);
        failingWrite = false;

        // A failed close is a failed SDL_PhysFS_WriteFile() too, as the data
        // may never have reached the file.
        failingFlush = true;
        SDL_assert(SDL_PhysFS_WriteFile("close.txt", "data", 4) == 0);
        SDL_assert(SDL_strstr(SDL_GetError(), "Failed to close file") != NULL);
        SDL_assert(SDL_strstr(SDL_GetError(), PHYSFS_getErrorByCode(PHYSFS_ERR_IO)) != NULL);

        // SDL_CloseIO() reports a failed close.
        {
            PHYSFS_File* handle = PHYSFS_openWrite("stream.txt");
            SDL_assert(handle != NULL);
            SDL_IOStream* io = SDL_PhysFS_OpenIO(handle);
            SDL_assert(io != NULL);
            SDL_assert(SDL_WriteIO(io, "data", 4) == 4);
            SDL_assert(SDL_CloseIO(io) == false);
            SDL_assert(SDL_strstr(SDL_GetError(), "Failed to close file") != NULL);
            SDL_assert(SDL_strstr(SDL_GetError(), PHYSFS_getErrorByCode(PHYSFS_ERR_IO)) != NULL);
        }
        failingFlush = false;

        // SDL_CloseIO() reports a failed flush of buffered data.
        {
            PHYSFS_File* handle = PHYSFS_openWrite("buffered.txt");
            SDL_assert(handle != NULL);
            SDL_assert(PHYSFS_setBuffer(handle, 64));
            SDL_IOStream* io = SDL_PhysFS_OpenIO(handle);
            SDL_assert(io != NULL);
            SDL_assert(SDL_WriteIO(io, "data", 4) == 4);
            failingWrite = true;
            SDL_assert(SDL_CloseIO(io) == false);
            SDL_assert(SDL_strstr(SDL_GetError(), "Failed to flush file") != NULL);
            SDL_assert(SDL_strstr(SDL_GetError(), PHYSFS_getErrorByCode(PHYSFS_ERR_IO)) != NULL);
            failingWrite = false;
        }

        // The handles that failed to close stay open in PhysFS, so it cannot
        // quit while they still fail to flush...
        failingFlush = true;
        SDL_assert(SDL_PhysFS_Quit() == false);

        // ...and closes them once they can be.
        failingFlush = false;
        SDL_assert(SDL_PhysFS_Quit() == true);

        SDL_assert(SDL_RemovePath(writeDir));
        SDL_free(writeDir);
    }

    SDL_Quit();

    return 0;
}
