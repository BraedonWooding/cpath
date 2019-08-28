#include "../../path.h"

#include <stdio.h>

void recursive_visit(cpath_dir *dir, int tab) {
    cpath_file file;
    while (cpathGetNextFile(dir, &file)) {
        for (int i = 0; i < tab; i++) putchar('\t');
        printf("%s\n", file.name);
        if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
            cpath_dir tmp;
            cpathFileToDir(&tmp, &file);
            recursive_visit(&tmp, tab + 1);
            cpathCloseDir(&tmp);
        }
    }
}

int main(void) {
    cpath_dir dir;
    cpath path = cpathGetCwd();
    cpathOpenDir(&dir, &path);
    recursive_visit(&dir, 0);
}