#define UNICODE

#include "path.h"

#include <stdio.h>

/*
    Output:
[D] .
[D] ..
[F] path.h
[D] a.out.dSYM
	[D] .
	[D] ..
	[D] Contents
		[D] .
		[D] ..
		[D] Resources
			[D] .
			[D] ..
			[D] DWARF
				[D] .
				[D] ..
				[F] a.out
		[F] Info.plist
[F] example.c
[F] a.out
*/

void recursive_visit(cpath_dir dir, int tab) {
    cpath_file file;
    while (cpathGetNextFile(&dir, &file)) {
        for (int i = 0; i < tab; i++) putchar('\t');
        printf("[%c] %s\n", (file.isDir ? 'D' : 'F'), file.name);
        // NOTE: you have to check for stuff like this:
        //       you can also write `!file.isSym` to just straight up
        //       avoid all symlinks which is probably a smart decision
        if (file.isDir && strcmp(file.name, CPATH_STRING("..")) &&
            strcmp(file.name, CPATH_STRING("."))) {
            cpath_dir tmp;
            cpathFileToDir(&tmp, &file);
            recursive_visit(tmp, tab + 1);
        }
    }
}

int main(void) {
    cpath_dir dir;
    char *cwd = cpathGetCwdAlloc();
    cpathOpenDir(&dir, cwd);
    recursive_visit(dir, 0);
    free(cwd);
}