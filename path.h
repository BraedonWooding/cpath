/*
MIT License

Copyright (c) 2019 Braedon Wooding

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CPATH_H
#define CPATH_H

/*
  Note this library also works with C++ via a C++ provided binding
  which is available for free just by including this on a C++ compiler
  or with __cplusplus defined

  If you want:
  - Unicode just add a #define CPATH_UNICODE or UNICODE or _UNICODE
  - Custom Allocators just #define:
    - CPATH_MALLOC and CPATH_FREE (make sure to define both)
*/

/*
  @TODO:
  - Windows has lifted their max path so we could use the new functions
    I don't think all functions have an alternative and it could be more messy
    still look into it eventually.
*/

// custom allocators
#if !defined CPATH_MALLOC && !defined CPATH_FREE
#define CPATH_MALLOC(size) malloc(size)
#define CPATH_FREE(ptr) free(ptr)
#elif (defined CPATH_MALLOC && !defined SIMPLED_DIR_FREE) || \
      (defined SIMPLED_DIR_FREE && !defined CPATH_MALLOC)
#error "Can't define only free or only malloc have to define both or neither"
#endif

// Support unicode
#if defined CPATH_UNICODE || (!defined UNICODE && defined _UNICODE)
#define UNICODE
#endif

#if defined CPATH_UNICODE || (defined UNICODE && !defined _UNICODE)
#define _UNICODE
#endif

#if !defined CPATH_UNICODE && (defined UNICODE || defined _UNICODE)
#define CPATH_UNICODE
#endif

#ifdef _MSC_VER
#define _CPATH_FUNC_ static __inline
#elif !defined __STDC_VERSION__ || __STDC_VERSION__ < 199901L
#define _CPATH_FUNC_ static __inline__
#else
#define _CPATH_FUNC_ static inline
#endif

#endif

/* == #includes == */

#if defined _MSC_VER_ || defined __MING32__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef _MSC_VER
#include <tchar.h>
// Ignore all warnings to do with std functions not being 100% safe
// they are fine
#pragma warning(push)
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <stddef.h>
#include <libgen.h>
#include <sys/stat.h>
#endif

// NOTE: This has to appear before defined BSD
//       since sys/param.h defines BSD given environment
#if defined __unix__ || (defined __APPLE__ && defined __MACH__)
#include <sys/param.h>
#endif

#if defined __linux__ || defined BSD
#include <limits.h>
#endif

#ifdef __MINGW32__
#include <tchar.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Linux has a max of 255 (+1 for \0) I couldn't find a max on windows
// But since 260 > 256 it is a reasonable value that should be crossplatform
#define CPATH_MAX_FILENAME_LEN (256)

/* Windows Unicode Support */
#if defined _MSC_VER || defined __MING32__

typedef TCHAR cpath_char_t;
#define CPATH_STRING(str) _TEXT(str)
#define cpath_str_length _tcslen
#define cpath_str_copy _tcscpy
#define cpath_str_concat _tcscat
#define cpath_str_find_last_char _tcsrchr
#define cpath_str_compare _tcsncmp

#else

typedef char cpath_char_t;
#define CPATH_STRING(str) str
#define cpath_str_length strlen
#define cpath_str_copy strcpy
#define cpath_str_concat strcat
#define cpath_str_find_last_char strrchr
#define cpath_str_compare strncmp

#endif

#if defined _MSC_VER_ || defined __MING32__
#define CPATH_MAX_PATH_LEN MAX_PATH
#elif defined __linux__ || defined BSD

#ifdef CPATH_MAX
#define CPATH_MAX_PATH_LEN CPATH_MAX
#endif

#endif

// Found this value online in a few other libraries
// so I'll just continue this tradition/standard despite not knowing why
// @A: This seems to be the max on Linux so I suppose its a reasonable value
#ifndef CPATH_MAX_PATH_LEN
#define CPATH_MAX_PATH_LEN (4096)
#endif

// on windows we need extra space for the \* mask that is required
// yes this is very weird...  Maybe we can just chuck it onto the max path?
// I don't think we can because it seems that it is independent of the max path
// i.e. if you can have a max path of 256 characters this includes the extra
//      in this case we could just subtract it from the max path
//      but then again we may not always want to apply the mask (and we don't)
//      so that isn't a great solution either...
// @TODO: Figure this stuff out.
#ifdef _MSC_VER
#define CPATH_PATH_EXTRA_CHARS (2)
#else
#define CPATH_PATH_EXTRA_CHARS (0)
#endif

/*
   NOTE: This isn't the maximum number of drives (that would be 26)
         This is the length of a drive prepath i.e. D:\ which is clearly 3

         No such max exists on other systems to the best of my knowledge

    By defining it like (static_assert(0, msg), 1) we make sure that we don't
    have a compiler syntax error in it's typical usage we prefer having
    the error from having the static assert rather than the error for having
    an incorrect token.
    @TODO: Do we actually need this???
*/
#if defined _MSC_VER || defined __MINGW32__
#define CPATH_MAX_DRIVE_LEN (3)
#elif __cpp_static_assert
#define CPATH_MAX_DRIVE_LEN \
    (static_assert(0, "You shouldn't use max drive length on non windows"), 1)
#elif __STDC_VERSION__ >= 201112L
#define CPATH_MAX_DRIVE_LEN \
    (_Static_assert(0, "You shouldn't use max drive length on non windows"), 1)
#else
#define CPATH_MAX_DRIVE_LEN \
    ("You shouldn't use max drive length on non windows", )
#endif

#if defined _WIN32
  #define _cpath_getcwd _getcwd
#else
  #include <unistd.h> // for getcwd()
  #define _cpath_getcwd getcwd
#endif

#if !defined _MSC_VER
#if defined __MING32__ && defined _UNICODE
typedef _WDIR cpath_dirdata_t;
typedef struct _wdirent cpath_dirent_t;
#define _cpath_opendir _wopendir
#define _cpath_readdir _wreaddir
#define _cpath_closedir _wclosedir
#else
typedef DIR cpath_dirdata_t;
typedef struct dirent cpath_dirent_t;
#define _cpath_opendir opendir
#define _cpath_readdir readdir
#define _cpath_closedir closedir
#endif
#endif

typedef cpath_char_t *cpath_str;
typedef int(*cpath_cmp)(const void*, const void*);

typedef struct cpath_file_t {
    int isDir;
    int isReg;

#if !defined _MSC_VER
#if defined __MING32__
    struct _stat stat;
#else
    struct stat stat;
#endif
#endif

  cpath_char_t path[CPATH_MAX_PATH_LEN];
  cpath_char_t name[CPATH_MAX_FILENAME_LEN];
  cpath_str extension;
} cpath_file;

typedef struct cpath_dir_t {
  cpath_file *files;
  size_t size;

#ifdef _MSC_VER
  HANDLE_ handle;
  WIN32_FIND_DATA findData;
#else
  cpath_dirdata_t *dir;
  cpath_dirent_t *dirent;
#endif

  // This is set whenever you do an emplace
  // This allows you to revert an emplace
  struct cpath_dir_t *parent;

  int hasNext;

  cpath_char_t path[CPATH_MAX_PATH_LEN];
  size_t pathLen;
} cpath_dir;

/* == Declarations == */

/*
  Get the current working directory allocating the space
*/
_CPATH_FUNC_
cpath_char_t *cpathGetCwdAlloc();

/*
  Get the current working directory passing in a buffer
*/
_CPATH_FUNC_
cpath_char_t *cpathGetCwdBuf(cpath_char_t *buf, size_t size);

/*
  Opens a directory placing information into the given directory buffer.
*/
_CPATH_FUNC_
int cpathOpenDir(cpath_dir *dir, const cpath_char_t *path);

/*
  Clear directory data.
*/
_CPATH_FUNC_
void cpathCloseDir(cpath_dir *dir);

/*
  Get the next file inside the directory, acts like an iterator.
  i.e. `while (cpathGetNextFile(...))` can use `dir->hasNext` to verify
  that there are indeed a file but it is safe!

  File may be null if you don't wish to actually get a copy
*/
_CPATH_FUNC_
int cpathGetNextFile(cpath_dir *dir, cpath_file *file);

/*
  Preload all files in a directory.  Will be called automatically in some
  function calls such as cpathGetFile()
*/
_CPATH_FUNC_
int cpathLoadAllFiles(cpath_dir *dir);

/*
  Get the `nth` file inside the directory, this is independent of the iterator
  above.

  Note calling this will require the reading of the entire directory.
       if you want to prevent all these allocations then I recommend using
       getNextFile(...)
*/
_CPATH_FUNC_
int cpathGetFile(cpath_dir *dir, cpath_file *file, size_t n);

/*
  Get a const reference to a file, this is more performant than GetFile
  but you can't modify anything about the object.
*/
_CPATH_FUNC_
int cpathGetFileConst(cpath_dir *dir, const cpath_file **file, size_t n);

/*
  More of a helper function, checks if we have space for `n`
  and will preload any files if required.
*/
_CPATH_FUNC_
int cpathCheckGetN(cpath_dir *dir, size_t n);

/*
  Opens the n'th sub directory into this directory
  Saves the old directory into the parent if given saveDir
*/
_CPATH_FUNC_
int cpathOpenSubDirEmplace(cpath_dir *dir, size_t n, int saveDir);

/*
  Restarts a directory to the beginning
*/
_CPATH_FUNC_
int cpathRestartDir(cpath_dir *dir);

/*
  Opens the n'th sub directory into other given directory
*/
_CPATH_FUNC_
int cpathOpenSubDir(
  cpath_dir *out, cpath_dir *dir, size_t n
);

/*
  Opens the next sub directory into other given directory
*/
_CPATH_FUNC_
int cpathOpenNextSubDir(
  cpath_dir *out, cpath_dir *dir, size_t n
);

/*
  Opens the next sub directory into this
  Saves the old directory into the parent if given saveDir
*/
_CPATH_FUNC_
int cpathOpenNextSubDirEmplace(cpath_dir *dir, size_t n, int saveDir);

/*
  Revert an emplace and go back to the parent.
  Note: This can occur multiple times.
*/
_CPATH_FUNC_
int cpathRevertEmplace(cpath_dir *dir);

/*
  Opens the given path as a file.
*/
_CPATH_FUNC_
int cpathOpenFile(cpath_file *file, const cpath_char_t *path);

/*
  Converts a given file to a directory.
  Note: Fails to convert if the file isn't a directory.
*/
_CPATH_FUNC_
int cpathFileToDir(cpath_dir *dir, const cpath_file *file);

/*
  Get the extension for a file.
  Note: You can't access a file's extension BEFORE you call this
        It will be null prior.
  Returns NULL if no extension or if it is a directory.
*/
_CPATH_FUNC_
cpath_str cpathGetExtension(cpath_file *file);

/*
  Standard compare function for files
*/
_CPATH_FUNC_
void cpathSort(cpath_dir *dir, cpath_cmp cmp);

/* == Definitions == */

/*
  Get the current working directory allocating the space
*/
_CPATH_FUNC_
cpath_char_t *cpathGetCwdAlloc() {
  cpath_char_t *buf;
  buf = (cpath_char_t*)CPATH_MALLOC(sizeof(cpath_char_t) * CPATH_MAX_PATH_LEN);
  return cpathGetCwdBuf(buf, CPATH_MAX_PATH_LEN);
}

_CPATH_FUNC_
cpath_char_t *cpathGetCwdBuf(cpath_char_t *buf, size_t size) {
  return _cpath_getcwd(buf, size);
}

_CPATH_FUNC_
int cpathOpenDir(cpath_dir *dir, const cpath_char_t *path) {
  const cpath_char_t *pathStr;
  size_t pathLen;

  if (dir == NULL || path == NULL || ((pathLen = cpath_str_length(path)) == 0)) {
    // empty strings are invalid arguments
    errno = EINVAL;
    return 0;
  }

  // remove trailing slashes then check path length
  pathStr = &path[pathLen - 1];
  while (pathLen > 0 &&
        (*pathStr == CPATH_STRING('\\') || *pathStr == CPATH_STRING('/'))) {
    pathStr--;
    pathLen--;
  }

  if (pathLen == 0) {
    errno = EINVAL;
    return 0;
  }

  if (pathLen + CPATH_PATH_EXTRA_CHARS >= CPATH_MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  dir->files = NULL;

#if defined _MSC_VER
  dir->handle = INVALID_HANDLE_VALUE;
#else
  dir->dir = NULL;
#endif
  dir->pathLen = pathLen;
  dir->path[pathLen] = CPATH_STRING('\0');
  dir->parent = NULL;
  dir->files = NULL;
#if defined _MSC_VER
  dir->handle = INVALID_HANDLE_VALUE;
#else
  dir->dir = NULL;
  dir->dirent = NULL;
#endif

  cpath_str dirPath = &dir->path[pathLen - 1];

  while (dirPath >= dir->path && pathStr >= path) {
    *dirPath = *pathStr;
    dirPath--;
    pathStr--;
  }

  return cpathRestartDir(dir);
}

_CPATH_FUNC_
int cpathRestartDir(cpath_dir *dir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  dir->hasNext = 1;
  dir->size = -1;
  if (dir->files != NULL) CPATH_FREE(dir->files);
  dir->files = NULL;

  // windows specific freeing
#if defined _MSC_VER
  if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
  dir->handle = INVALID_HANDLE_VALUE;
#else
  if (dir->dir != NULL) _cpath_closedir(dir->dir);
  dir->dir = NULL;
  dir->dirent = NULL;
#endif

  if (dir->parent != NULL) {
    cpathCloseDir(dir->parent);
    dir->parent = NULL;
  }

#if defined _MSC_VER
  cpath_char_t pathBuf[CPATH_MAX_PATH_LEN];
  cpath_str_copy(pathBuf, dir->path);
  cpath_str_cat(pathBuf, CPATH_STRING("\\*"));

#if (defined WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
  dir->handle = FindFirstFileEx(path_buf, FindExInfoStandard, &dir->findData,
                                FindExSearchNameMatch, NULL, 0);
#else
  dir->handle = FindFirstFile(path_buf, &dir->findData);
#endif

  if (dir->handle == INVALID_HANDLE_VALUE) {
    errno = ENOENT;
    // free associate memory and exit
    cpathCloseDir(dir);
    return 0;
  }

#else

  dir->dir = _cpath_opendir(dir->path);
  if (dir->dir == NULL) {
    cpathCloseDir(dir);
    return 0;
  }
  dir->dirent = _cpath_readdir(dir->dir);
  // empty directory
  if (dir->dirent == NULL) dir->hasNext = 0;

#endif

  return 1;
}

_CPATH_FUNC_
void cpathCloseDir(cpath_dir *dir) {
  if (dir == NULL) return;

  cpathRestartDir(dir);
  memset(dir->path, 0, sizeof(dir->path));
}

int cpathMoveNextFile(cpath_dir *dir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }
  if (!dir->hasNext) {
    return 0;
  }

#if defined _MSC_VER
  if (FindNextFile(dir->handle, &dir->findData) == 0) {
    dir->hasNext = 0;
    if (GetLastError() != ERROR_SUCCESS &&
        GetLastError() != ERROR_NO_MORE_FILES) {
      cpathCloseDir(dir);
      errno = EIO;
      return 0;
    }
  }
#else
  dir->dirent = _cpath_readdir(dir->dir);
  if (dir->dirent == NULL) {
    dir->hasNext = 0;
  }
#endif

  return 1;
}

_CPATH_FUNC_
int cpathGetNextFile(cpath_dir *dir, cpath_file *file) {
  if (file != NULL) {
    // load current file into file
    const cpath_char_t *filename;
    size_t filenameLen;
#if defined _MSC_VER
    if (dir->handle == INVALID_HANDLE_VALUE) {
      return 0;
    }
    filename = dir->fileData.cFileName;
    filenameLen = cpath_str_length(filename);
#else
    if (dir->dirent == NULL) {
      return 0;
    }
    filename = dir->dirent->d_name;
    filenameLen = dir->dirent->d_namlen;
#endif

    size_t totalLen = dir->pathLen + filenameLen;
    if (totalLen + 1 + CPATH_PATH_EXTRA_CHARS >= CPATH_MAX_PATH_LEN ||
        filenameLen >= CPATH_MAX_FILENAME_LEN) {
      errno = ENAMETOOLONG;
      return 0;
    }

    cpath_str_copy(file->name, filename);
    cpath_str_copy(file->path, dir->path);
    cpath_str_concat(file->path, CPATH_STRING("/"));
    cpath_str_concat(file->path, filename);

#if !defined _MSC_VER
#if defined __MING32__
    if (_tstat(file->path, &file->stat) == -1) {
      return 0;
    }
#elif defined _BSD_SOURCE || defined _DEFAULT_SOURCE	\
  || (defined _XOPEN_SOURCE && _XOPEN_SOURCE >= 500)	\
  || (defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 200112L)
    if (lstat(file->path, &file->stat) == -1) {
      return 0;
    }
#else
    if (stat(file->path, &file->stat) == -1) {
      return 0;
    }
#endif
#endif
    cpathGetExtension(file);

#ifdef _MSC_VER
    file->isDir = !!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    if (dir->_f.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
      file->isReg = 1;
    } else if (!(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) &&
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
#ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM) &&
#endif
#ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA) &&
#endif
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) &&
               !(dir->_f.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)) {
      file->isReg = 1;
    } else {
      file->isReg = 0;
    }
#else
    file->isDir = S_ISDIR(file->stat.st_mode);
    file->isReg = S_ISREG(file->stat.st_mode);
#endif
  }

  errno = 0;
  if (dir->hasNext && !cpathMoveNextFile(dir) && errno != 0) {
    return 0;
  }

  return 1;
}

_CPATH_FUNC_
int cpathLoadAllFiles(cpath_dir *dir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  // @Ugly:
  /*
    The problem is that we could go through all files to get a count
    This is kinda ugly but is linear and ensures we don't make allocations
    The other method is to use a 'vector' however the copies are really
    expensive since we have huge objects which is why I chose the count method
  */
  size_t count = 0;
  errno = 0;
  while (cpathMoveNextFile(dir)) {
    count++;
  }

  // we can have 0 files
  if (count == 0) {
    dir->size = 0;
    return 1;
  }

  // loading failed for some reason or we failed to restart the dir iterator
  if (errno != 0 || !cpathRestartDir(dir)) {
    // we won't close the directory though!
    // we'll just pretend we have no files!
    dir->size = 0;
    return 0;
  }

  dir->size = count;
  dir->files = (cpath_file*) CPATH_MALLOC(sizeof(cpath_file) * count);
  if (dir->files == NULL) {
    // we won't close the directory just error out
    dir->size = 0;
    return 0;
  }

  errno = 0;
  int i = 0;
  // also make sure that we don't overflow the array
  // because size requirements changed i.e. race condition
  // we could fix this by resizing but that's super expensive
  // maybe this is time for a linked list or just to dynamically allocate
  // all file nodes and waste the extra 8 bytes per (and cache locality)
  while (i < dir->size && cpathGetNextFile(dir, &dir->files[i])) {
    i++;
  }

  if (i < dir->size) {
    // we stopped early due to error I'm still not going to crazily error out
    // because it may just be race condition and I'll be a bit lazy
    // @TODO: check if it was just race or if a read failed
    dir->size = i;
  }

  return 1;
}

_CPATH_FUNC_
int cpathCheckGetN(cpath_dir *dir, size_t n) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  if (dir->size == -1) {
    // we haven't loaded
    cpathLoadAllFiles(dir);
  }

  if (n >= dir->size) {
    errno = ENOENT;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathGetFile(cpath_dir *dir, cpath_file *file, size_t n) {
  if (file == NULL) {
    errno = EINVAL;
    return 0;
  }
  if (!cpathCheckGetN(dir, n)) return 0;

  cpathGetExtension(&dir->files[n]);
  memcpy(file, &dir->files[n], sizeof(cpath_file));
  return 1;
}

_CPATH_FUNC_
int cpathGetFileConst(cpath_dir *dir, const cpath_file **file, size_t n) {
  if (dir == NULL || file == NULL) {
    errno = EINVAL;
    return 0;
  }

  if (dir->size == -1) {
    // we haven't loaded
    cpathLoadAllFiles(dir);
  }

  if (n >= dir->size) {
    errno = ENOENT;
    return 0;
  }

  cpathGetExtension(&dir->files[n]);
  *file = &dir->files[n];
  return 1;
}

_CPATH_FUNC_
int cpathOpenSubDirEmplace(cpath_dir *dir, size_t n, int saveDir);

_CPATH_FUNC_
int cpathOpenSubDir(
  cpath_dir *out, cpath_dir *dir, size_t n
);

_CPATH_FUNC_
int cpathOpenNextSubDir(
  cpath_dir *out, cpath_dir *dir, size_t n
);

_CPATH_FUNC_
int cpathOpenNextSubDirEmplace(cpath_dir *dir, size_t n, int saveDir);

_CPATH_FUNC_
int cpathRevertEmplace(cpath_dir *dir);

_CPATH_FUNC_
int cpathOpenFile(cpath_file *file, const cpath_char_t *path);

_CPATH_FUNC_
int cpathFileToDir(cpath_dir *dir, const cpath_file *file) {
  if (dir == NULL || file == NULL || !file->isDir) {
    errno = EINVAL;
    return 0;
  }
  return cpathOpenDir(dir, file->path);
}

_CPATH_FUNC_
cpath_str cpathGetExtension(cpath_file *file) {
  if (file->extension != NULL) return file->extension;

  cpath_char_t *dot = cpath_str_find_last_char(file->name, CPATH_STRING('.'));
  if (dot != NULL) {
    // extension
    file->extension = dot + 1;
  } else {
    // no extension so set to '\0'
    file->extension = &file->name[cpath_str_length(file->name)];
  }
  return file->extension;
}

_CPATH_FUNC_
void cpathSort(cpath_dir *dir, cpath_cmp cmp) {
  if (dir->files == NULL) {
    if (dir->size == -1) {
      // bad, this means you try to sorted a directory before you preloaded
      // in this case we'll just preload the files.  This is just to be nice
      cpathLoadAllFiles(dir);
    }

    // this is fine just means no files in directory
    if (dir->size == 0) {
      return;
    }
  }
  qsort(dir->files, dir->size, sizeof(struct cpath_dir_t), cmp);
}
