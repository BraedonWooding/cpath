#include "../../../cpath.h"

using namespace cpath;

#include <cstdio>

void emplace(Dir &dir) {
  int tab = 0;
  do {
    while (Opt<File, Error::Type> file = dir.GetNextFile()) {
      for (int i = 0; i < tab; i++) putchar('\t');
      puts(file->Name());
      if (file->IsDir() && !file->IsSpecialHardLink()) {
        dir.OpenSubFileEmplace(**file, true);
        tab++;
      }
    }
    tab--;
  } while (dir.RevertEmplace());
}

int main(void) {
  Path cwd = Path(".");
  Dir dir = Dir(cwd);
  emplace(dir);
}