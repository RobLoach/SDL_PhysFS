#ifndef SDL_PHYSFS_H__
#define SDL_PHYSFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>

SDL_bool SDL_PhysFS_Init();
SDL_bool SDL_PhysFS_Quit();
SDL_bool SDL_PhysFS_Mount(const char* newDir, const char* mountPoint);
SDL_bool SDL_PhysFS_MountFromMemory(const unsigned char *fileData, int dataSize, const char* newDir, const char* mountPoint);
SDL_bool SDL_PhysFS_Unmount(const char* oldDir);
SDL_RWops* SDL_PhysFS_RWFromFile(const char* filename);
SDL_Surface* SDL_PhysFS_LoadBMP(const char* filename);
SDL_AudioSpec* SDL_PhysFS_LoadWAV(const char* filename, SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len);
void* SDL_PhysFS_LoadFile(const char* filename, size_t *datasize);

#ifdef __cplusplus
}
#endif

#endif  // SDL_PHYSFS_H__

#ifdef SDL_PHYSFS_IMPLEMENTATION
#ifndef SDL_PHYSFS_IMPLEMENTATION_ONCE
#define SDL_PHYSFS_IMPLEMENTATION_ONCE

#ifdef __cplusplus
extern "C" {
#endif

#include "physfs.h"

int SDL_PhysFS_SetLastError(const char* functionName) {
    int error = PHYSFS_getLastErrorCode();
    SDL_SetError("%s: %s", functionName, PHYSFS_getErrorByCode(error));
    return error;
}

/**
 * Initialize the PhysFS virtual file system.
 *
 * @return SDL_TRUE on success, SDL_FALSE otherwise.
 *
 * @see SDL_PhysFS_Quit()
 */
SDL_bool SDL_PhysFS_Init() {
    if (PHYSFS_init(0) == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_Init");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/**
 * Close the PhysFS virtual file system.
 *
 * @return SDL_TRUE on success, SDL_FALSE otherwise.
 */
SDL_bool SDL_PhysFS_Quit() {
    if (PHYSFS_deinit() == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_Quit");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/**
 * Mounts the given directory, at the given mount point.
 *
 * @param newDir Directory or archive to add to the path, in platform-dependent notation.
 * @param mountPoint Location in the interpolated tree that this archive will be "mounted", in platform-independent notation. NULL or "" is equivalent to "/".
 *
 * @return SDL_TRUE on success, SDL_FALSE otherwise.
 *
 * @see SDL_PhysFS_Unmount()
 */
SDL_bool SDL_PhysFS_Mount(const char* newDir, const char* mountPoint) {
    if (PHYSFS_mount(newDir, mountPoint, 1) == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_Mount");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/**
 * Mounts the given file data as a mount point in PhysFS.
 *
 * @param fileData The archive data as a file buffer.
 * @param dataSize The size of the file buffer.
 * @param newDir A filename that can represent the file data. Has to be unique. For example: data.zip
 * @param mountPoint The location in the tree that the archive will be mounted.
 *
 * @return SDL_TRUE on success, SDL_FALSE otherwise.
 *
 * @see SDL_PhysFS_Mount()
 */
SDL_bool SDL_PhysFS_MountFromMemory(const unsigned char *fileData, int dataSize, const char* newDir, const char* mountPoint) {
    if (dataSize <= 0) {
        SDL_SetError("SDL_PhysFS_MountFromMemory: Cannot mount a data size of 0");
        return SDL_FALSE;
    }

    if (PHYSFS_mountMemory(fileData, (PHYSFS_uint64)dataSize, 0, newDir, mountPoint, 1) == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_MountFromMemory");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

/**
 * Unmounts the given directory or archive.
 *
 * @param oldDir The directory that was supplied to MountPhysFS's newDir.
 *
 * @return SDL_TRUE on success, SDL_FALSE otherwise.
 *
 * @see MountPhysFS()
 */
SDL_bool SDL_PhysFS_Unmount(const char* oldDir) {
    if (PHYSFS_unmount(oldDir) == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_Unmount");
        return SDL_FALSE;
    }

    return SDL_TRUE;
}

Sint64 SDLCALL SDL_PhysFS_RWopsSize(struct SDL_RWops *rw) {
    PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
    return (Sint64) PHYSFS_fileLength(handle);
}

Sint64 SDLCALL SDL_PhysFS_RWopsSeek(struct SDL_RWops *rw, Sint64 offset, int whence) {
    PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
    PHYSFS_sint64 pos = 0;

    if (whence == RW_SEEK_SET) {
        pos = (PHYSFS_sint64) offset;
    }
    else if (whence == RW_SEEK_CUR) {
        const PHYSFS_sint64 current = PHYSFS_tell(handle);
        if (current == -1) {
            return SDL_SetError("Can't find position in file: %s",
                          PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }

        if (offset == 0) {
            return (Sint64) current;
        }

        pos = current + ((PHYSFS_sint64) offset);
    }
    else if (whence == RW_SEEK_END) {
        const PHYSFS_sint64 len = PHYSFS_fileLength(handle);
        if (len == -1) {
            return SDL_SetError("Can't find end of file: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }

        pos = len + ((PHYSFS_sint64) offset);
    }
    else {
        return SDL_SetError("Invalid 'whence' parameter.");
    }

    if (pos < 0) {
        return SDL_SetError("Attempt to seek past start of file.");
    }
    
    if (!PHYSFS_seek(handle, (PHYSFS_uint64)pos)) {
        return SDL_SetError("PhysicsFS error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return (Sint64) pos;
}

size_t SDL_PhysFS_RWopsRead(struct SDL_RWops *rw, void *ptr, size_t size, size_t maxnum) {
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    const PHYSFS_uint64 readlen = (PHYSFS_uint64) (maxnum * size);
    const PHYSFS_sint64 rc = PHYSFS_readBytes(handle, ptr, readlen);
    if (rc != ((PHYSFS_sint64) readlen)) {
        if (!PHYSFS_eof(handle)){
            return (size_t)SDL_SetError("PhysicsFS error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }
    } 

    return (size_t) rc / size;
}

size_t SDLCALL SDL_PhysFS_RWopsWrite(struct SDL_RWops *rw, const void *ptr, size_t size, size_t num) {
    PHYSFS_File *handle = (PHYSFS_File *) rw->hidden.unknown.data1;
    const PHYSFS_uint64 writelen = (PHYSFS_uint64) (num * size);
    const PHYSFS_sint64 rc = PHYSFS_writeBytes(handle, ptr, writelen);
    if (rc != ((PHYSFS_sint64) writelen)) {
        SDL_SetError("PhysicsFS error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return (size_t) rc;
}

static int SDL_PhysFS_RWopsClose(SDL_RWops *rw) {
    if (rw == NULL) {
        return 0;
    }

    PHYSFS_File *handle = (PHYSFS_File *)rw->hidden.unknown.data1;
    if (handle != NULL) {
        if (!PHYSFS_close(handle)) {
            return SDL_SetError("PhysicsFS error: %s", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        }

        SDL_FreeRW(rw);
    }

    return 0;
}

SDL_RWops *SDL_PhysFS_RWopsCreate(PHYSFS_File *handle) {
    SDL_RWops *retval = NULL;

    if (handle == NULL) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_RWopsCreate");
    }
    else {
        retval = SDL_AllocRW();
        if (retval != NULL) {
            retval->size  = SDL_PhysFS_RWopsSize;
            retval->seek  = SDL_PhysFS_RWopsSeek;
            retval->read  = SDL_PhysFS_RWopsRead;
            retval->write = SDL_PhysFS_RWopsWrite;
            retval->close = SDL_PhysFS_RWopsClose;
            retval->hidden.unknown.data1 = handle;
        }
    }

    return retval;
}

SDL_RWops* SDL_PhysFS_RWFromFile(const char* filename) {
    PHYSFS_File* handle = PHYSFS_openRead(filename);
    if (handle == NULL) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_LoadRWops");
        return NULL;
    }
    return SDL_PhysFS_RWopsCreate(handle);
}

SDL_Surface* SDL_PhysFS_LoadBMP(const char* filename) {
    SDL_RWops* rwops = SDL_PhysFS_RWFromFile(filename);
    if (rwops == NULL) {
        return NULL;
    }
    return SDL_LoadBMP_RW(rwops, 1);
}

SDL_AudioSpec* SDL_PhysFS_LoadWAV(const char* filename, SDL_AudioSpec * spec, Uint8 ** audio_buf, Uint32 * audio_len) {
    SDL_RWops* rwops = SDL_PhysFS_RWFromFile(filename);
    if (rwops == NULL) {
        return NULL;
    }
    return SDL_LoadWAV_RW(rwops, 1, spec, audio_buf, audio_len);
}

void* SDL_PhysFS_LoadFile(const char* filename, size_t *datasize) {
    void* handle = PHYSFS_openRead(filename);
    if (handle == 0) {
        SDL_PhysFS_SetLastError("SDL_PhysFS_LoadFile");
        *datasize = 0;
        return 0;
    }

    // Check to see how large the file is.
    PHYSFS_sint64 size = PHYSFS_fileLength(handle);
    if (size <= 0) {
        *datasize = 0;
        PHYSFS_close(handle);
        return 0;
    }

    // Read the file, return if it's empty.
    void* buffer = SDL_malloc((size_t)size);
    PHYSFS_sint64 read = PHYSFS_readBytes(handle, buffer, (PHYSFS_uint64)size);
    if (read < 0) {
        *datasize = 0;
        SDL_free(buffer);
        SDL_PhysFS_SetLastError("SDL_PhysFS_LoadFile");
        PHYSFS_close(handle);
        return 0;
    }

    // Close the file handle, and return the bytes read and the buffer.
    PHYSFS_close(handle);
    *datasize = (size_t)read;
    return buffer;
}

#ifdef __cplusplus
}
#endif

#endif  // SDL_PHYSFS_IMPLEMENTATION_ONCE
#endif  // SDL_PHYSFS_IMPLEMENTATION