#include <stdio.h>
#include "../../others/tinydir.h"

void recurse(tinydir_dir *dir, int tab) {
  tinydir_file file;
  while (dir->has_next) {
    if (tinydir_readfile(dir, &file) == -1) {
      perror("Error getting file");
      tinydir_close(dir);
      return;
    }

    printf("%s\n", file.name);
    if (file.is_dir && _tinydir_strcmp(file.name, ".") && _tinydir_strcmp(file.name, "..")) {
      tinydir_dir tmp;
      tinydir_open(&tmp, file.path);
      recurse(&tmp, tab + 1);
      tinydir_close(&tmp);
    }

    if (tinydir_next(dir) == -1) {
      perror("Error getting file");
      tinydir_close(dir);
      return;
    }
  }
}

int main(void) {
  tinydir_dir dir;
  if (tinydir_open(&dir, ".") == -1)
  {
      perror("Error opening file");
      tinydir_close(&dir);
      return 0;
  }
  recurse(&dir, 0);

  tinydir_close(&dir);
}
