#include "cpath.h"

using namespace cpath;

#include <iostream>
#include <iomanip>

void recursive_visit(Dir &dir, int tab) {
  while (Opt<File, Error::Type> file = dir.GetNextFile()) {
    for (int i = 0; i < tab; i++) std::cout << '\t';
    ByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
    std::cout << "[" << (file->IsDir() ? 'D' : 'F') << "] "
              << file->Name() << " " << std::fixed << std::setprecision(1)
              << file->GetFileSizeDec(1024) << " "
              << file->GetFileSizeSuffix(flags) << std::endl;
    if (file->IsDir() && !file->IsSpecialHardLink()) {
      Opt<Dir, Error::Type> dir = file->ToDir();
      recursive_visit(**dir, tab + 1);
      dir->Close();
    }
  }
}

void emplace(Dir &dir) {
  /*
  //     This method uses just one object and a few saved directories to iterate
  //     This is safe from stackoverflow but will require dynamic allocation
  // */
  int tab = 0;
  do {
    while (Opt<File, Error::Type> file = dir.GetNextFile()) {
      for (int i = 0; i < tab; i++) std::cout << '\t';
      ByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
      std::cout << "[" << (file->IsDir() ? 'D' : 'F') << "] "
                << file->Name() << " " << std::fixed << std::setprecision(1)
                << file->GetFileSizeDec(1024) << " "
                << file->GetFileSizeSuffix(flags) << std::endl;
      if (file->IsDir() && !file->IsSpecialHardLink()) {
        dir.OpenSubFileEmplace(**file, true);
        tab++;
      }
    }
    tab--;
  } while (dir.RevertEmplace());
}

int main(void) {
  Path cwd = Path::GetCwd();
  Dir dir = Dir(cwd);
  recursive_visit(dir, 0);
  emplace(dir);

  cwd /= "a.out";
  Opt<File, Error::Type> file = File::OpenFile(cwd);
  ByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
  std::cout << "[" << (file->IsDir() ? 'D' : 'F') << "] "
            << file->Name() << " " << std::fixed << std::setprecision(1)
            << file->GetFileSizeDec(1024) << " "
            << file->GetFileSizeSuffix(flags) << std::endl;
}
