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
        CPathByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
        printf("[%c] %s %.1lf %s\n", (file.isDir ? 'D' : 'F'), file.name,
            cpathGetFileSizeDec(&file, 1000), cpathGetFileSizePrefix(&file, flags));
        if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
            cpath_dir tmp;
            cpathFileToDir(&tmp, &file);
            // recursive_visit(tmp, tab + 1);
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