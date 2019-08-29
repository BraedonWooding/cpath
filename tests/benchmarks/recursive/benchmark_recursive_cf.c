#ifdef _WIN32
#include <Windows.h>
#endif

#define CUTE_FILES_IMPLEMENTATION
#include "../../others/cute_files.h"

#include <stdio.h>

void print_dir(cf_file_t* file, void* udata) {
  puts(file->path);
}

int main(void) {
  cf_traverse(".", print_dir, NULL);
  return 0;
}
