#include <stdio.h>
#include "../../../path.h"

void emplace(cpath_dir *dir) {
  cpath_file file;
  int tab = 0;

  while (dir != NULL) {
    while (cpathGetNextFile(dir, &file)) {
      for (int i = 0; i < tab; i++) putchar('\t');
      printf("%s\n", file.name);
      if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
        cpathOpenSubFileEmplace(dir, &file, 1);
        tab++;
      }
    }
    cpathRevertEmplace(&dir);
    tab--;
  }
}

int main(void) {
  cpath_dir dir;
  cpath path;
  cpathFromStr(&path, ".");
  cpathOpenDir(&dir, &path);

  emplace(&dir);
}