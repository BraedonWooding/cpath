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