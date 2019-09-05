#include "../cpath.h"

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

#include "../../../cpath.h"

using namespace cpath;

#include <cstdio>

void recursive_visit(Dir &dir, int tab) {
  while (Opt<File, Error::Type> file = dir.GetNextFile()) {
    for (int i = 0; i < tab; i++) putchar('\t');
    puts(file->Name());
    if (file->IsDir() && !file->IsSpecialHardLink()) {
      Opt<Dir, Error::Type> dir = file->ToDir();
      recursive_visit(**dir, tab + 1);
      dir->Close();
    }
  }
}

int main(void) {
  // an empty path is always `.`
  Path cwd = Path();
  Dir dir = Dir(cwd);
  recursive_visit(dir, 0);
}