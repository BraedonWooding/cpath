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

void recursive_visit(cpath_dir *dir, int tab) {
    cpath_file file;
    while (cpathGetNextFile(dir, &file)) {
        for (int i = 0; i < tab; i++) putchar('\t');
        CPathByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
        printf("[%c] %s %.1lf %s\n", (file.isDir ? 'D' : 'F'), file.name,
            cpathGetFileSizeDec(&file, 1000), cpathGetFileSizePrefix(&file, flags));
        if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
            cpath_dir tmp;
            cpathFileToDir(&tmp, &file);
            recursive_visit(&tmp, tab + 1);
            cpathCloseDir(&tmp);
        }
    }
}

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
            CPathByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
            printf("[%c] %s %.1lf %s\n", (file.isDir ? 'D' : 'F'), file.name,
                cpathGetFileSizeDec(&file, 1000), cpathGetFileSizePrefix(&file, flags));
            if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
                cpathOpenSubFileEmplace(dir, &file, 1);
                tab++;
            }
        }
        cpathRevertEmplace(&dir);
        tab--;
    }
}

void paths_example() {
    // casts are automatic and should be crossplatform
    // if you want to make it truly unicode independent and its a literal
    // then you can use CPATH_STR() which will convert it to the wide version
    // as required.  This of course only applies to string literals
    // and you can use cpathFromString(myStr, path) to write to paths
    cpath path = cpathGetCwd();
    CPATH_CONCAT_LIT(&path, "a.out");

    // if you want crossplatform
    if (cpathExists(&path)) {
        printf("Exists\n");
        // path exists and the real name is
        // cpath real;
        // // cpathCanonicalise(path, &real);
        // // you can map it to a string
        // // this is very platform dependent... and won't work with unicode
        // printf("%s\n", (char*)real);
        // // it is safer to do
        // char *str = cpathToUtf8(real);
        // // or more performant to do
        // cpathPrint(real);
    }
}

int main(void) {
    cpath_dir dir;
    cpath path = cpathGetCwd();
    cpathOpenDir(&dir, &path);
    recursive_visit(&dir, 0);
    paths_example();
    emplace(&dir);

    cpath_file file;
    CPATH_CONCAT_LIT(&path, "a.out");
    cpathOpenFile(&file, &path);
    CPathByteRep flags = BYTE_REP_JEDEC | BYTE_REP_BYTE_WORD;
    printf("[%c] %s %.1lf %s\n", (file.isDir ? 'D' : 'F'), file.name,
        cpathGetFileSizeDec(&file, 1000), cpathGetFileSizePrefix(&file, flags));
}