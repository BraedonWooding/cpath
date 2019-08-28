#include <stdio.h>
#include "../../path.h"

void emplace(cpath_dir *dir) {
    /*
        This method uses just one object and a few saved directories to iterate
        This is safe from stackoverflow but will require dynamic allocation
    */
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
    cpath path = cpathGetCwd();
    cpathOpenDir(&dir, &path);
    // recursive_visit(&dir, 0);
    // paths_example();

    emplace(&dir);
}