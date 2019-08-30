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

#ifdef __cplusplus
extern "C" {
#endif

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
  Note on why this library offers no guarantees on reentrant systems
  or no TOUTOC (and other race conditions):
  - Often supporting this requires breaking a ton of compatability
    and often the behaviour of each of the individual commands differs way
    too much to make it consistent.  Even when fully following it we can't
    guarantee that you won't just misuse it and still cause the race conditions
  - The majority (99%) of cases simply just don't care about it how often
    are your files being written over...
  - We have extra safety on things like opening directories and recursive
    solutions to make sure that things like that don't happen.
  - How do you want to handle it properly?  You may want to just keep going
    or you may want some way to unwind what you previously did, or just take
    a snapshot of the system... It is too varied for us to offer a generalised
    way.
  - The majority of libraries don't offer it or they offer it to an extremely
    limited number of commands with the others not having it, in my opinion
    this is worse than just not offering it.  Atleast we are consistent with
    our guarantees
  - File Systems are a mess and already are basically a gigantic global mess
    trying to guarantee any sort of safety is not only a complexity mess but
    also can give the wrong idea.
  - You should be able to detect hard failures due to the change in a folder
    and simply just re-call.  Of course this could happen repeatedly but
    come on... In reality it will occur once in a blue moon.
  - Reloading files is easy it is just `cpathLoadFiles`
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
#elif (defined CPATH_MALLOC && !defined CPATH_FREE) || \
      (defined CPATH_FREE && !defined CPATH_MALLOC)
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

/* == #includes == */

#if defined _MSC_VER || defined __MINGW32__
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
#if defined _MSC_VER || defined __MINGW32__
#define CPATH_STR(str) _TEXT(str)
#define cpath_str_length _tcslen
#define cpath_str_copy _tcscpy
#define cpath_strn_copy _tcsncpy
#define cpath_str_concat _tcscat
#define cpath_str_find_last_char _tcsrchr
#define cpath_str_compare _tcscmp
#define cpath_str_compare_safe _tcsncmp
#else
#define CPATH_STR(str) str
#define cpath_str_length strlen
#define cpath_str_copy strcpy
#define cpath_strn_copy strncpy
#define cpath_str_concat strcat
#define cpath_str_find_last_char strrchr
#define cpath_str_compare strcmp
#define cpath_str_compare_safe strncmp
#endif

#if defined _MSC_VER || defined __MINGW32__
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
#if defined _MSC_VER
#define FILE_IS(f, flag) !!(f.dwFileAttributes & FILE_ATTRIBUTE_##flag)
#define FILE_IS_NOT(f, flag) !(f.dwFileAttributes & FILE_ATTRIBUTE_##flag)
#endif

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
#if defined __MINGW32__ && defined _UNICODE
#define _cpath_opendir _wopendir
#define _cpath_readdir _wreaddir
#define _cpath_closedir _wclosedir
#else
#define _cpath_opendir opendir
#define _cpath_readdir readdir
#define _cpath_closedir closedir
#endif
#endif

#if !defined CPATH_NO_CPP_BINDINGS && defined __cplusplus
  namespace cpath { namespace internals {
#endif

#if defined _MSC_VER || defined __MINGW32__
// todo
typedef cpath_offset_t;
typedef cpath_time_t;
typedef TCHAR cpath_char_t;
#else
typedef char cpath_char_t;
typedef off_t cpath_offset_t;
typedef time_t cpath_time_t;
#endif

#if !defined _MSC_VER
#if defined __MINGW32__ && defined _UNICODE
typedef _WDIR cpath_dirdata_t;
typedef struct _wdirent cpath_dirent_t;
#else
typedef DIR cpath_dirdata_t;
typedef struct dirent cpath_dirent_t;
#endif
#endif

typedef cpath_char_t *cpath_str;
typedef int(*cpath_err_handler)();
typedef int(*cpath_cmp)(const void*, const void*);
typedef struct cpath_t {
  cpath_char_t buf[CPATH_MAX_PATH_LEN];
  size_t len;
} cpath;

typedef struct cpath_file_t {
  int isDir;
  int isReg;
  int isSym;

#if !defined _MSC_VER
#if defined __MINGW32__
    struct _stat stat;
#else
    struct stat stat;
#endif
#endif

  int statLoaded;

  cpath path;
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

  cpath path;
} cpath_dir;

// This allows a forced type
typedef uint8_t CPathByteRep;
enum CPathByteRep_ {
  BYTE_REP_JEDEC          = 0,  // KB = 1024, ...
  BYTE_REP_DECIMAL        = 1,  // 1000 interval segments, B, kB, MB, GB, ...
  BYTE_REP_IEC            = 2,  // KiB = 1024, ...
  BYTE_REP_DECIMAL_UPPER  = 3,  // 1000 interval segments but B, KB, MB, GB, ...
  BYTE_REP_DECIMAL_LOWER  = 4,  // 1000 interval segments but b, kb, mb, gb, ...

  // representations
  BYTE_REP_LONG           = 0b10000000, // Represent as words i.e. kibibytes
  BYTE_REP_BYTE_WORD      = 0b01000000, // Just `B` as `Byte` (only bytes)
};

typedef void(*cpath_traverse_it)(
  cpath_file *file, cpath_dir *parent, int depth,
  void *data
);

/* == Declarations == */

/* == Path == */



/* == File System == */

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
int cpathOpenDir(cpath_dir *dir, const cpath *path);

/*
  Clear directory data.
  If closing parents then will recurse through all previous emplaces.
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
  Opens the next sub directory into this
  Saves the old directory into the parent if given saveDir
*/
_CPATH_FUNC_
int cpathOpenSubFileEmplace(cpath_dir *dir, const cpath_file *file,int saveDir);

/*
  Opens the n'th sub directory into other given directory
*/
_CPATH_FUNC_
int cpathOpenSubDir(cpath_dir *out, cpath_dir *dir, size_t n);

/*
  Opens the next sub directory into other given directory
*/
_CPATH_FUNC_
int cpathOpenNextSubDir(cpath_dir *out, cpath_dir *dir);

/*
  Opens the next sub directory into this
  Saves the old directory into the parent if given saveDir
*/
_CPATH_FUNC_
int cpathOpenNextSubDirEmplace(cpath_dir *dir, int saveDir);

/*
  Revert an emplace and go back to the parent.
  Note: This can occur multiple times.
  Returns true if it went back to the parent.
*/
_CPATH_FUNC_
int cpathRevertEmplace(cpath_dir **dir);

/*

*/
_CPATH_FUNC_
int cpathRevertEmplaceCopy(cpath_dir *dir);

/*
  Opens the given path as a file.
*/
_CPATH_FUNC_
int cpathOpenFile(cpath_file *file, const cpath *path);

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

/*
  Is this file either `.` or `..`
*/
_CPATH_FUNC_
int cpathFileIsSpecialHardLink(const cpath_file *file);

/*
  Get the time of last access
*/
_CPATH_FUNC_
cpath_time_t cpathGetLastAccess(cpath_file *file);

/*
  Get the time of last modification
*/
_CPATH_FUNC_
cpath_time_t cpathGetLastModification(cpath_file *file);

/*
  Get the time of last access as a formatted string
*/
_CPATH_FUNC_
char *cpathGetLastAccessStr(cpath_file *file);

/*
  Get the time of last modification as a formatted string
*/
_CPATH_FUNC_
char *cpathGetLastModificationStr(cpath_file *file);

/*
  Get the file size in bytes of this file.
*/
_CPATH_FUNC_
cpath_offset_t cpathGetFileSize(cpath_file *file);

/*
  Get the file size prefix
*/
_CPATH_FUNC_
const cpath_char_t *cpathGetFileSizeSuffix(cpath_file *file, CPathByteRep rep);

/*
  Get the file size in decimal form
*/
_CPATH_FUNC_
double cpathGetFileSizeDec(cpath_file *file, int interval);

_CPATH_FUNC_
void cpathWriteCwd(cpath *path);
/* == Definitions == */

/* == Path == */

_CPATH_FUNC_
void cpathTrim(cpath *path) {
  // trim all the terminating `/` and `\`
  while (path->buf[path->len - 1] == CPATH_STR('/') ||
         path->buf[path->len - 1] == CPATH_STR('\\')) {
    path->buf[path->len - 1] = '\0';
    path->len--;
  }
}

_CPATH_FUNC_
cpath cpathFromUtf8(const char *str) {
  cpath path;
  if (str[0] == '\0') {
    // empty string which is just '.'
    path.len = 1;
    path.buf[0] = CPATH_STR('.');
    path.buf[1] = '\0';
    return path;
  }

  path.len = 0;
  path.buf[0] = '\0';
  size_t len = cpath_str_length(str);
  // NOTE: max path len includes the `\0` where as str length doesn't!
  if (len >= CPATH_MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return path;
  }
#if defined CPATH_UNICODE && defined _MSC_VER
  mbstowcs_s(&path.len, path.buf, len + 1, str, CPATH_MAX_PATH_LEN);
#else
  // this is slightly cheaper since it won't keep going till the end of buffer
  cpath_strn_copy(path.buf, str, len + 1);
#endif
  path.len = len;
  cpathTrim(&path);
  return path;
}

_CPATH_FUNC_
void cpathCopy(cpath *out, const cpath *in) {
  cpath_str_copy(out->buf, in->buf);
  out->len = in->len;
}

#define CPATH_CONCAT_LIT(path, other) cpathConcatStr(path, CPATH_STR(other))

_CPATH_FUNC_
int cpathFromStr(cpath *out, const cpath_char_t *str) {
  size_t len = cpath_str_length(str);
  if (len >= CPATH_MAX_PATH_LEN) return 0;
  if (len == 0) {
    out->len = 1;
    out->buf[0] = CPATH_STR('.');
    out->buf[1] = '\0';
    return 1;
  }

  out->len = len;
  cpath_strn_copy(out->buf, str, len);
  return 1;
}

_CPATH_FUNC_
int cpathConcatStrn(cpath *out, const cpath_char_t *other, size_t len) {
  if (len + out->len >= CPATH_MAX_PATH_LEN) {
    // path too long, >= cause max path includes '\0'
    errno = ENAMETOOLONG;
    return 0;
  }

  if (other[0] != CPATH_STR('/') && out->buf[out->len - 1] != CPATH_STR('/')) {
    out->buf[out->len++] = '/';
    out->buf[out->len] = '\0';
  }

  cpath_str_concat(out->buf, other);
  out->len += len;
  cpathTrim(out);
  return 1;
}

_CPATH_FUNC_
int cpathConcatStr(cpath *out, const cpath_char_t *other) {
  return cpathConcatStrn(out, other, cpath_str_length(other));
}

_CPATH_FUNC_
int cpathConcat(cpath *out, const cpath *other) {
  return cpathConcatStrn(out, other->buf, other->len);
}

_CPATH_FUNC_
int cpathExists(const cpath *path) {
#if defined _MSC_VER || defined __MINGW32__
  DWORD res = GetFileAttributes(path->buf);
  return res != INVALID_FILE_ATTRIBUTES;
#else
  /*
    We can either try to use stat or just access, access is more efficient
    when not checking permissions.

    struct stat tmp;
    return stat(path->buf, &tmp) == 0;
  */
  return access(path->buf, F_OK) != -1;
#endif
}

// attempts to canonicalise without file system help
_CPATH_FUNC_
int cpathCanonicaliseFake(cpath *path) {
  if (path == NULL) {
    errno = EINVAL;
    return 0;
  }

  cpath buf;
  buf.len = 0;
  cpath_char_t *chr = &path->buf[0];
  while (*chr != '\0') {
    if (*chr == CPATH_STR('.')) {
      if (chr[1] == CPATH_STR('.')) {
        if (buf.len == 0) {
          // no directory to go back based on string alone
          errno = ENOENT;
          return 0;
        }
        // remove last directory ignoring the last `/` since that is part of ..
        buf.len--;
        while (buf.len > 0 && buf.buf[buf.len - 1] != CPATH_STR('/') &&
               buf.buf[buf.len - 1] != CPATH_STR('\\')) {
          buf.len--;
        }
        // skip twice
        chr++;
        chr++;
      } else if (chr[1] == CPATH_STR('/') || chr[1] == '\0' ||
                 chr[1] == CPATH_STR('\\')) {
        // skip
        chr++;
      } else {
        buf.buf[buf.len++] = *chr;
      }
    } else {
      buf.buf[buf.len++] = *chr;
    }
    chr++;
  }

  buf.buf[buf.len] = '\0';
  cpathCopy(path, &buf);
  cpathTrim(path);
  if (path->len == 0) {
    path->buf[0] = '.';
    path->buf[1] = '\0';
    path->len = 1;
  }

  return 1;
}

_CPATH_FUNC_
void cpathClear(cpath *path) {
  path->buf[0] = '\0';
  path->len = 0;
}

/* == File System == */

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
void cpathWriteCwd(cpath *path) {
  cpathGetCwdBuf(path->buf, CPATH_MAX_PATH_LEN);
  path->len = cpath_str_length(path->buf);
  cpathTrim(path);
}

_CPATH_FUNC_
cpath cpathGetCwd() {
  cpath path;
  cpathWriteCwd(&path);
  cpathTrim(&path);
  return path;
}

_CPATH_FUNC_
int cpathOpenDir(cpath_dir *dir, const cpath *path) {
  if (dir == NULL || path == NULL || path->len == 0) {
    // empty strings are invalid arguments
    errno = EINVAL;
    return 0;
  }

  if (path->len + CPATH_PATH_EXTRA_CHARS >= CPATH_MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  dir->files = NULL;

#if defined _MSC_VER
  dir->handle = INVALID_HANDLE_VALUE;
#else
  dir->dir = NULL;
#endif
  dir->path.buf[path->len] = CPATH_STR('\0');
  dir->parent = NULL;
  dir->files = NULL;
#if defined _MSC_VER
  dir->handle = INVALID_HANDLE_VALUE;
#else
  dir->dir = NULL;
  dir->dirent = NULL;
#endif

  cpathCopy(&dir->path, path);
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

#if defined _MSC_VER
  if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
  dir->handle = INVALID_HANDLE_VALUE;
#else
  if (dir->dir != NULL) _cpath_closedir(dir->dir);
  dir->dir = NULL;
  dir->dirent = NULL;
#endif

  // Ignore parent, just restart this dir

#if defined _MSC_VER
  cpath_char_t pathBuf[CPATH_MAX_PATH_LEN];
  cpath_str_copy(pathBuf, dir->path);
  cpath_str_cat(pathBuf, CPATH_STR("\\*"));

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

  dir->dir = _cpath_opendir(dir->path.buf);
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

  dir->hasNext = 1;
  dir->size = -1;
  if (dir->files != NULL) CPATH_FREE(dir->files);
  dir->files = NULL;

#if defined _MSC_VER
  if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
  dir->handle = INVALID_HANDLE_VALUE;
#else
  if (dir->dir != NULL) _cpath_closedir(dir->dir);
  dir->dir = NULL;
  dir->dirent = NULL;
#endif

  cpathClear(&dir->path);
  if (dir->parent != NULL) {
    cpathCloseDir(dir->parent);
    CPATH_FREE(dir->parent);
    dir->parent = NULL;
  }
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
int cpathGetFileInfo(cpath_file *file) {
  if (file->statLoaded) {
    return 1;
  }
#if !defined _MSC_VER
#if defined __MINGW32__
  if (_tstat(file->path.buf, &file->stat) == -1) {
    return 0;
  }
#elif defined _BSD_SOURCE || defined _DEFAULT_SOURCE	\
      || (defined _XOPEN_SOURCE && _XOPEN_SOURCE >= 500)	\
      || (defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 200112L)
  if (lstat(file->path.buf, &file->stat) == -1) {
    return 0;
  }
#else
  if (stat(file->path.buf, &file->stat) == -1) {
    return 0;
  }
#endif
#endif
  file->statLoaded = 1;
  return 1;
}

_CPATH_FUNC_
int cpathLoadFlags(cpath_dir *dir, cpath_file *file, void *data) {
#if defined _MSC_VER
  WIN32_FIND_DATA *find = (WIN32_FIND_DATA*) data;
  file->isDir = FILE_IS(find, DIRECTORY);
  if (FILE_IS(find, NORMAL)) {
    file->isReg = 1;
  } else if (FILE_IS_NOT(find, DEVICE) && FILE_IS_NOT(find, DIRECTORY) &&
             FILE_IS_NOT(find, ENCRYPTED) && FILE_IS_NOT(find, OFFLINE) &&
#ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
             FILE_IS_NOT(find, INTEGRITY_STREAM) &&
#endif
#ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
             FILE_IS_NOT(find, NO_SCRUB_DATA) &&
#endif
             FILE_IS_NOT(find, TEMPORARY)) {
    file->isReg = 1;
  } else {
    file->isReg = 0;
  }
  file->isSym = FILE_IS(find, REPARSE_POINT);
#else
  if (dir->dirent == NULL || dir->dirent->d_type == DT_UNKNOWN) {
    if (!cpathGetFileInfo(file)) {
      return 0;
    }

    file->isDir = S_ISDIR(file->stat.st_mode);
    file->isReg = S_ISREG(file->stat.st_mode);
    file->isSym = S_ISLNK(file->stat.st_mode);
  } else {
    file->isDir = dir->dirent->d_type == DT_DIR;
    file->isReg = dir->dirent->d_type == DT_REG;
    file->isSym = dir->dirent->d_type == DT_LNK;
    file->statLoaded = 0;
  }
#endif
  return 1;
}

_CPATH_FUNC_
int cpathPeekNextFile(cpath_dir *dir, cpath_file *file) {
  if (file == NULL || dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  file->statLoaded = 0;
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
  size_t totalLen = dir->path.len + filenameLen;
  if (totalLen + 1 + CPATH_PATH_EXTRA_CHARS >= CPATH_MAX_PATH_LEN ||
      filenameLen >= CPATH_MAX_FILENAME_LEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  cpath_str_copy(file->name, filename);
  cpathCopy(&file->path, &dir->path);
  CPATH_CONCAT_LIT(&file->path, "/");
  cpathConcatStr(&file->path, filename);
  cpathGetExtension(file);
#if defined _MSC_VER
  cpathLoadFlags(dir, file, dir->fileData)
#else
  cpathLoadFlags(dir, file, NULL);
#endif
  return 1;
}

_CPATH_FUNC_
int cpathGetNextFile(cpath_dir *dir, cpath_file *file) {
  if (file != NULL) {
    if (!cpathPeekNextFile(dir, file)) {
      return 0;
    }
  }

  errno = 0;
  if (dir->hasNext && !cpathMoveNextFile(dir) && errno != 0) {
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathFileIsSpecialHardLink(const cpath_file *file) {
  return (file->name[0] == '.' && (file->name[1] == '\0' ||
         (file->name[1] == '.' && file->name[2] == '\0')));
}

_CPATH_FUNC_
int cpathLoadAllFiles(cpath_dir *dir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  if (dir->files != NULL) CPATH_FREE(dir->files);

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
int cpathOpenSubFileEmplace(cpath_dir *dir, const cpath_file *file,
                            int saveDir) {
  cpath_dir *saved = NULL;
  if (saveDir) {
    // save the old one
    saved = (cpath_dir*)CPATH_MALLOC(sizeof(cpath_dir));
    if (saved != NULL) memcpy(saved, dir, sizeof(cpath_dir));
  }

  if (!cpathFileToDir(dir, file)) {
    if (saved != NULL) CPATH_FREE(saved);
    return 0;
  }

  dir->parent = saved;

  return 1;
}

_CPATH_FUNC_
int cpathOpenSubDirEmplace(cpath_dir *dir, size_t n, int saveDir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }
  if (!cpathCheckGetN(dir, n)) return 0;

  const cpath_file *file;
  if (!cpathGetFileConst(dir, &file, n) ||
      !file->isDir || !cpathOpenSubFileEmplace(dir, file, saveDir)) {
    if (!file->isDir) errno = EINVAL;
    return 0;
  }

  return 1;
}

_CPATH_FUNC_
int cpathOpenSubDir(cpath_dir *out, cpath_dir *dir, size_t n) {
  if (dir == NULL || out == NULL) {
    errno = EINVAL;
    return 0;
  }
  if (!cpathCheckGetN(dir, n)) return 0;

  const cpath_file *file;
  if (!cpathGetFileConst(dir, &file, n) ||
      !file->isDir || !cpathFileToDir(out, file)) {
    if (!file->isDir) errno = EINVAL;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathOpenNextSubDir(cpath_dir *out, cpath_dir *dir) {
  if (dir == NULL || out == NULL) {
    errno = EINVAL;
    return 0;
  }

  cpath_file file;
  if (!cpathGetNextFile(dir, &file) ||
      !file.isDir || !cpathFileToDir(out, &file)) {
    if (!file.isDir) errno = EINVAL;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathOpenCurrentSubDir(cpath_dir *out, cpath_dir *dir) {
  if (dir == NULL || out == NULL) {
    errno = EINVAL;
    return 0;
  }

  cpath_file file;
  if (!cpathPeekNextFile(dir, &file) ||
      !file.isDir || !cpathFileToDir(out, &file)) {
    if (!file.isDir) errno = EINVAL;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathOpenNextSubDirEmplace(cpath_dir *dir, int saveDir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  cpath_file file;
  if (!cpathGetNextFile(dir, &file) ||
      !file.isDir || !cpathOpenSubFileEmplace(dir, &file, saveDir)) {
    if (!file.isDir) errno = EINVAL;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathOpenCurrentSubDirEmplace(cpath_dir *dir, int saveDir) {
  if (dir == NULL) {
    errno = EINVAL;
    return 0;
  }

  cpath_file file;
  if (!cpathPeekNextFile(dir, &file) ||
      !file.isDir || !cpathOpenSubFileEmplace(dir, &file, saveDir)) {
    if (!file.isDir) errno = EINVAL;
    return 0;
  }
  return 1;
}

_CPATH_FUNC_
int cpathRevertEmplace(cpath_dir **dir) {
  if (dir == NULL || *dir == NULL) return 0;
  cpath_dir *tmp = (*dir)->parent;
  (*dir)->parent = NULL;
  cpathCloseDir(*dir);
  *dir = tmp;
  return *dir != NULL;
}

_CPATH_FUNC_
int cpathRevertEmplaceCopy(cpath_dir *dir) {
  if (dir == NULL) return 0;
  cpath_dir *tmp = dir->parent;
  dir->parent = NULL;
  cpathCloseDir(dir);
  if (tmp != NULL) {
    memcpy(dir, tmp, sizeof(cpath_dir));
  }
  return tmp != NULL;
}

_CPATH_FUNC_
int cpathOpenFile(cpath_file *file, const cpath *path) {
  // We want to efficiently open this file so unlike most libraries
  // we won't use a directory search we will just find the given file
  // or directly use stuff like dirname/basename!
  cpath_dir dir;
  int found = 0;

  if (file == NULL || path == NULL || path->len == 0) {
    errno = EINVAL;
    return 0;
  }
  if (path->len >= CPATH_MAX_PATH_LEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  void *data;
  void *handle = NULL;

  cpathCopy(&file->path, path);

#if defined _MSC_VER
  WIN32_FIND_DATA findData;

#if (defined WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
  handle = FindFirstFileEx(path->buf, FindExInfoStandard, &findData,
                                FindExSearchNameMatch, NULL, 0);
#else
  handle = FindFirstFile(path->buf, &findData);
#endif

  if (handle == INVALID_HANDLE_VALUE) {
    errno = ENOENT;
    return 0;
  }

  data = &findData;
  cpath_str_copy(file->name, findData.cFileName);
#else
  // copy the name then strip to just basename
  // this is very ewwwww, honestly we probably would be better
  // to just do this ourselves since ugh
  cpath_strn_copy(file->name, path->buf, path->len);
  char *tmp = basename(file->name);
  // some systems allocate it seems.. ugh
  if (tmp != file->name) {
    cpath_str_copy(file->name, tmp);
    CPATH_FREE(tmp);
  }
  // @TODO: make sure tmp isn't too long
#endif
  dir.dirent = NULL;
  file->statLoaded = 0;
  int res = cpathLoadFlags(&dir, file, data);
#if defined _MSC_VER
  FindClose((HANDLE)handle);
#endif

  return res;
}

_CPATH_FUNC_
int cpathFileToDir(cpath_dir *dir, const cpath_file *file) {
  if (dir == NULL || file == NULL || !file->isDir) {
    errno = EINVAL;
    return 0;
  }
  return cpathOpenDir(dir, &file->path);
}

_CPATH_FUNC_
cpath_str cpathGetExtension(cpath_file *file) {
  if (file->extension != NULL) return file->extension;

  cpath_char_t *dot = cpath_str_find_last_char(file->name, CPATH_STR('.'));
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
cpath_time_t cpathGetLastAccess(cpath_file *file) {
  if (!file->statLoaded) cpathGetFileInfo(file);

#if defined _MSC_VER || defined __MINGW32__
  // Idk todo
#else
  return file->stat.st_atime;
#endif
}

_CPATH_FUNC_
cpath_time_t cpathGetLastModification(cpath_file *file) {
  if (!file->statLoaded) cpathGetFileInfo(file);

#if defined _MSC_VER || defined __MINGW32__
  // Idk todo
#else
  return file->stat.st_mtime;
#endif
}

_CPATH_FUNC_
cpath_offset_t cpathGetFileSize(cpath_file *file) {
  if (!file->statLoaded) cpathGetFileInfo(file);

#if defined _MSC_VER || defined __MINGW32__
  // Idk todo
#else
  return file->stat.st_size;
#endif
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

static const cpath_char_t *prefixTableDecimal[] = {
  CPATH_STR("kB"), CPATH_STR("MB"), CPATH_STR("GB"), CPATH_STR("TB"),
  CPATH_STR("PB"), CPATH_STR("EB"), CPATH_STR("ZB"), CPATH_STR("YB"),
};
static const cpath_char_t *prefixTableDecimalUpper[] = {
  CPATH_STR("KB"), CPATH_STR("MB"), CPATH_STR("GB"), CPATH_STR("TB"),
  CPATH_STR("PB"), CPATH_STR("EB"), CPATH_STR("ZB"), CPATH_STR("YB"),
};
static const cpath_char_t *prefixTableDecimalLower[] = {
  CPATH_STR("kb"), CPATH_STR("mb"), CPATH_STR("gb"), CPATH_STR("tb"),
  CPATH_STR("pb"), CPATH_STR("eb"), CPATH_STR("zb"), CPATH_STR("yb"),
};
static const cpath_char_t *prefixTableIEC[] = {
  CPATH_STR("KiB"), CPATH_STR("MiB"), CPATH_STR("GiB"), CPATH_STR("TiB"),
  CPATH_STR("PiB"), CPATH_STR("EiB"), CPATH_STR("ZiB"), CPATH_STR("YiB"),
};
// identical to decimal upper but kept separate for the sake of readability
static const cpath_char_t *prefixTableJEDEC[] = {
  CPATH_STR("KB"), CPATH_STR("MB"), CPATH_STR("GB"), CPATH_STR("TB"),
  CPATH_STR("PB"), CPATH_STR("EB"), CPATH_STR("ZB"), CPATH_STR("YB"),
};

_CPATH_FUNC_
double cpathGetFileSizeDec(cpath_file *file, int intervalSize) {
  double size = cpathGetFileSize(file);
  int steps = 0;
  while (size >= intervalSize / 2 && steps < 8) {
    size /= intervalSize;
    steps++;
  }
  return size;
}

_CPATH_FUNC_
const cpath_char_t *cpathGetFileSizeSuffix(cpath_file *file, CPathByteRep rep) {
  cpath_offset_t size = cpathGetFileSize(file);
  int word = (rep & BYTE_REP_LONG) == BYTE_REP_LONG;
  int byte_word = (rep & BYTE_REP_BYTE_WORD) == BYTE_REP_BYTE_WORD;
  // disable both them to make comparing easier
  rep &= ~BYTE_REP_LONG;
  rep &= ~BYTE_REP_BYTE_WORD;
  int interval = rep == BYTE_REP_IEC || rep == BYTE_REP_JEDEC ? 1024 : 1000;

  if (size < interval / 2) {
    // then we just have a byte case
    if (word || byte_word) {
      return rep != BYTE_REP_DECIMAL_LOWER ? CPATH_STR("Bytes")
                                           : CPATH_STR("bytes");
    } else {
      return rep != BYTE_REP_DECIMAL_LOWER ? CPATH_STR("B")
                                           : CPATH_STR("b");
    }
  }

  int steps = 0;
  double size_flt = size;
  while (size_flt >= interval / 2 && steps < 8) {
    size_flt /= (double)interval;
    steps++;
  }

  switch (rep) {
    case BYTE_REP_IEC:            return prefixTableIEC[steps - 1];
    case BYTE_REP_JEDEC:          return prefixTableJEDEC[steps - 1];
    case BYTE_REP_DECIMAL:        return prefixTableDecimal[steps - 1];
    case BYTE_REP_DECIMAL_LOWER:  return prefixTableDecimalLower[steps - 1];
    case BYTE_REP_DECIMAL_UPPER:  return prefixTableDecimalUpper[steps - 1];
    default: return NULL;
  }
}

_CPATH_FUNC_
void cpath_traverse(
  cpath_dir *dir, int depth, int visit_subdirs, cpath_err_handler err,
  cpath_traverse_it it, void *data
) {
  // currently implemented recursively
  if (dir == NULL) {
    errno = EINVAL;
    if (err != NULL) err();
    return;
  }
  cpath_file file;
  while (cpathGetNextFile(dir, &file)) {
    if (it != NULL) {
      it(&file, dir, depth, data);
    }
    if (file.isDir && visit_subdirs && !cpathFileIsSpecialHardLink(&file)) {
      cpath_dir tmp;
      if (!cpathFileToDir(&tmp, &file)) {
        if (err) err();
        continue;
      }
      cpath_traverse(&tmp, depth + 1, visit_subdirs, err, it, data);
      cpathCloseDir(&tmp);
    }
  }
}

#endif
#ifdef __cplusplus
}
#ifndef CPATH_NO_CPP_BINDINGS
  } }

// cpp bindings
namespace cpath {
// using is a C++11 extension, we want to remain pretty
// compatible to all versions
typedef internals::cpath_time_t     Time;
typedef internals::cpath_offset_t   Offset;
typedef internals::cpath_char_t     RawChar;
typedef internals::cpath            RawPath;
typedef internals::cpath_dir        RawDir;
typedef internals::cpath_file       RawFile;
typedef internals::CPathByteRep     ByteRep;

typedef void(*TraversalIt)(
  struct File &file, struct Dir &parent, int depth, void *data
);

typedef void(*ErrorHandler)();

/*
  This has to be kept to date with the other representation
  Kinda disgusting but seems to be the best way to ensure encapsulation
*/
const ByteRep BYTE_REP_JEDEC          = internals::BYTE_REP_JEDEC;
const ByteRep BYTE_REP_DECIMAL        = internals::BYTE_REP_DECIMAL;
const ByteRep BYTE_REP_IEC            = internals::BYTE_REP_IEC;
const ByteRep BYTE_REP_DECIMAL_UPPER  = internals::BYTE_REP_DECIMAL_UPPER;
const ByteRep BYTE_REP_DECIMAL_LOWER  = internals::BYTE_REP_DECIMAL_LOWER;
const ByteRep BYTE_REP_LONG           = internals::BYTE_REP_LONG;
const ByteRep BYTE_REP_BYTE_WORD      = internals::BYTE_REP_BYTE_WORD;

#if !defined _MSC_VER
#if defined __MINGW32__
typedef struct _stat RawStat;
#else
typedef struct stat RawStat;
#endif
#endif

template<typename T, typename Err>
struct Opt {
private:
#if __cplusplus <= 199711L
  struct Data {
#else
  union Data {
#endif
  T raw;
  Err err;

  Data(T raw) : raw(raw) {}
  Data(Err err) : err(err) {}
} data;
  bool ok;

public:
  Opt(T raw) : data(raw), ok(true) {}
  Opt(Err err) : data(err), ok(false) {}
  Opt() : data(Err()), ok(false) {}

  bool IsOk() const {
    return ok;
  }

  operator bool() const {
    return ok;
  }

  Err GetErr() const {
    return data.err;
  }

  T GetRaw() const {
    return data.raw;
  }

  T *operator*() {
    if (!ok) return NULL;
    return &data.raw;
  }

  T *operator->() {
    if (!ok) return NULL;
    return &data.raw;
  }
};

struct Error {
  enum Type {
    UNKNOWN,
    INVALID_ARGUMENTS,
    NAME_TOO_LONG,
    NO_SUCH_FILE,
    IO_ERROR,
  };

  static Type FromErrno() {
    switch (errno) {
      case EINVAL: return INVALID_ARGUMENTS;
      case ENAMETOOLONG: return NAME_TOO_LONG;
      case ENOENT: return NO_SUCH_FILE;
      case EIO: return IO_ERROR;
      default: return UNKNOWN;
    }
  }

  static void WriteToErrno(Type type) {
    switch (type) {
      case INVALID_ARGUMENTS: errno = EINVAL;
      case NAME_TOO_LONG: errno = ENAMETOOLONG;
      case NO_SUCH_FILE: errno = ENOENT;
      case IO_ERROR: errno = IO_ERROR;
      default: errno = 0;
    }
  }
};

struct Path {
private:
  RawPath path;

public:
  inline Path(const char *str) : path(internals::cpathFromUtf8(str)) { }

#if CPATH_UNICODE
  inline Path(RawChar *str) {
    internals::cpathFromStr(&path, str);
  }
#endif

  inline Path() {
    path.len = 1;
    path.buf[0] = CPATH_STR('.');
    path.buf[1] = '\0';
  }

  inline Path(RawPath path) : path(path) {}

  static inline Path GetCwd() {
    return Path(internals::cpathGetCwd());
  }

  inline bool Exists() const {
    return internals::cpathExists(&path);
  }

  inline const RawChar *GetBuffer() const {
    return path.buf;
  }

  inline const RawPath *GetRawPath() const {
    return &path;
  }

  inline unsigned long Size() const {
    return path.len;
  }

  inline friend bool operator==(const Path &p1, const Path &p2) {
    if (p1.path.len != p2.path.len) return false;
    return !cpath_str_compare_safe(p1.path.buf, p2.path.buf, p1.path.len);
  }

  inline friend bool operator!=(const Path &p1, const Path &p2) {
    return !(p1 == p2);
  }

  inline Path &operator/=(const Path &other) {
    internals::cpathConcat(&path, &other.path);
    return *this;
  }

  inline Path &operator/=(const RawChar *str) {
    internals::cpathConcatStr(&path, str);
    return *this;
  }

#if CPATH_UNICODE
  inline Path &operator/=(const char *str) {
    internals::cpathConcatStr(&path, str);
    return *this;
  }
#endif

  inline friend Path operator/(Path lhs, const Path &rhs) {
    lhs /= rhs;
    return lhs;
  }

  inline friend Path operator/(Path lhs, const char *&rhs) {
    lhs /= rhs;
    return lhs;
  }

#if CPATH_UNICODE
  inline friend Path operator/(Path lhs, const RawChar *&rhs) {
    lhs /= rhs;
    return lhs;
  }
#endif
};

struct File {
private:
  RawFile file;
  bool hasArg;

public:
  inline bool LoadFileInfo() {
    return internals::cpathGetFileInfo(&file);
  }

  inline bool LoadFlags(struct Dir &dir, void *data);

  inline RawFile *GetRawFile() {
    return &file;
  }

  inline const RawFile *GetRawFileConst() const {
    return &file;
  }

  inline File(RawFile file) : file(file), hasArg(true) {}
  inline File() : hasArg(false) {}

  inline static Opt<File, Error::Type> OpenFile(const Path &path) {
    RawFile file;
    if (!internals::cpathOpenFile(&file, path.GetRawPath())) {
      return Error::FromErrno();
    } else {
      return File(file);
    }
  }

  inline Opt<Dir, Error::Type> ToDir() const;

  inline bool IsSpecialHardLink() const {
    if (!hasArg) {
      errno = EINVAL;
      return false;
    }
    return internals::cpathFileIsSpecialHardLink(&file);
  }

  inline bool IsDir() const {
    if (!hasArg) {
      errno = EINVAL;
      return false;
    }
    return file.isDir;
  }

  inline bool IsReg() const {
    if (!hasArg) {
      errno = EINVAL;
      return false;
    }
    return file.isReg;
  }

  inline bool IsSym() const {
    if (!hasArg) {
      errno = EINVAL;
      return false;
    }
    return file.isSym;
  }

  inline const RawChar *Extension() {
    if (internals::cpathGetExtension(&file)) {
      return file.extension;
    } else {
      return NULL;
    }
  }

  inline Time GetLastAccess() {
    return internals::cpathGetLastAccess(&file);
  }

  inline Time GetLastModification() {
    return internals::cpathGetLastModification(&file);
  }

  inline Time GetFileSize() {
    return internals::cpathGetFileSize(&file);
  }

  inline double GetFileSizeDec(int intervalSize) {
    return internals::cpathGetFileSizeDec(&file, intervalSize);
  }

  inline const RawChar *GetFileSizeSuffix(ByteRep rep) {
    return internals::cpathGetFileSizeSuffix(&file, rep);
  }

  inline Path Path() const {
    return cpath::Path(file.path);
  }

  inline const RawChar *Name() {
    return file.name;
  }

#if !defined _MSC_VER
#if defined __MINGW32__
  inline RawStat Stat() {
    if (!file.statLoaded) internals::cpathGetFileInfo(&file);
    return file.stat;
  }
#else
  inline RawStat Stat() {
    if (!file.statLoaded) internals::cpathGetFileInfo(&file);
    return file.stat;
  }
#endif
#endif
};

struct Dir {
private:
  RawDir dir;
  bool loadedFiles;

public:
  inline Dir(const Path &path) {
    loadedFiles = false;
    internals::cpathOpenDir(&dir, path.GetRawPath());
  }
  inline Dir(RawDir dir) : dir(dir) {
    loadedFiles = false;
  }
  inline Dir() {
    loadedFiles = false;
    internals::cpathOpenDir(&dir, Path().GetRawPath());
  }

  inline static Opt<Dir, Error::Type> Open(const File &file) {
    RawDir dir;
    internals::cpathFileToDir(&dir, file.GetRawFileConst());
    return Dir(dir);
  }

  inline void Traverse(TraversalIt it, ErrorHandler err, int visitSubDir,
                       int depth, void *data) {
    while (Opt<File, Error::Type> file = GetNextFile()) {
      if (file && it) it(**file, *this, depth, data);
      if (!file) {
        Error::WriteToErrno(file.GetErr());
        if (err) err();
        continue;
      }

      if (file->IsDir() && !file->IsSpecialHardLink()) {
        Opt<Dir, Error::Type> dir = file->ToDir();
        if (!dir) {
          Error::WriteToErrno(dir.GetErr());
          if (err) err();
          continue;
        }
        dir->Traverse(it, err, visitSubDir, depth + 1, data);
        dir->Close();
      }
    }
  }

  inline bool OpenEmplace(const Path &path) {
    internals::cpathCloseDir(&dir);
    loadedFiles = false;
    return internals::cpathOpenDir(&dir, path.GetRawPath());
  }

  inline void Close() {
    return internals::cpathCloseDir(&dir);
  }

  inline void Sort(internals::cpath_cmp cmp) {
    loadedFiles = true; // will load files as required
    internals::cpathSort(&dir, cmp);
  }

  inline bool MoveNext() {
    return internals::cpathMoveNextFile(&dir);
  }

  inline RawDir *GetRawDir() {
    return &dir;
  }

  inline Opt<File, Error::Type> PeekNextFile() {
    RawFile file;
    if (!internals::cpathPeekNextFile(&dir, &file)) {
      return Error::FromErrno();
    } else {
      return File(file);
    }
  }

  inline Opt<File, Error::Type> GetNextFile() {
    RawFile file;
    if (!internals::cpathGetNextFile(&dir, &file)) {
      return Error::FromErrno();
    } else {
      return File(file);
    }
  }

  inline bool LoadFiles() {
    loadedFiles = true;
    return internals::cpathLoadAllFiles(&dir);
  }

  inline unsigned long Size() {
    if (!loadedFiles && !LoadFiles()) return 0;
    return dir.size;
  }

  inline Opt<File, Error::Type> GetFile(unsigned long n) {
    RawFile file;
    if (!internals::cpathGetFile(&dir, &file, n)) {
      return Error::FromErrno();
    } else {
      return File(file);
    }
  }

  inline bool OpenSubFileEmplace(const File &file, bool saveDir) {
    loadedFiles = false;
    return internals::cpathOpenSubFileEmplace(&dir, file.GetRawFileConst(), saveDir);
  }

  inline bool OpenSubDirEmplace(unsigned int n, bool saveDir) {
    loadedFiles = false;
    return internals::cpathOpenSubDirEmplace(&dir, n, saveDir);
  }

  inline Opt<Dir, Error::Type> OpenSubDir(unsigned int n) {
    RawDir raw;
    if (!internals::cpathOpenSubDir(&raw, &dir, n)) {
      return Error::FromErrno();
    } else {
      return Dir(raw);
    }
  }

  inline Dir OpenNextSubDir() {
    RawDir raw;
    internals::cpathOpenNextSubDir(&raw, &dir);
    return Dir(raw);
  }

  inline Dir OpenCurrentSubDir() {
    RawDir raw;
    internals::cpathOpenCurrentSubDir(&raw, &dir);
    return Dir(raw);
  }

  inline bool OpenNextSubDirEmplace(bool saveDir) {
    loadedFiles = false;
    return internals::cpathOpenNextSubDirEmplace(&dir, saveDir);
  }

  inline bool OpenCurrentSubDirEmplace(bool saveDir) {
    loadedFiles = false;
    return internals::cpathOpenCurrentSubDirEmplace(&dir, saveDir);
  }

  inline bool RevertEmplace() {
    return internals::cpathRevertEmplaceCopy(&dir);
  }
};

bool File::LoadFlags(struct Dir &dir, void *data) {
  return internals::cpathLoadFlags(dir.GetRawDir(), &file, data);
}

Opt<Dir, Error::Type> File::ToDir() const {
  return Dir::Open(*this);
}

}

#endif
#endif
