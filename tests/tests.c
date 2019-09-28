// This file should be run with -DCPATH_UNICODE on and off

#define CUTE_FILES_IMPLEMENTATION

#include "../cpath.h"
#include "others/cute_files.h"
#include "others/tinydir.h"

#define OBS_STRCMP cpath_str_compare

#include "cbench.h"
#include "obsidian.h"
#include "obsidian_extras.h"

void recursive_visit(cpath_dir *dir, int tab) {
  cpath_file file;
  while (cpathGetNextFile(dir, &file)) {
    for (int i = 0; i < tab; i++)
      putchar('\t');
    printf("%s\n", file.name);
    if (file.isDir && !cpathFileIsSpecialHardLink(&file)) {
      cpath_dir tmp;
      cpathFileToDir(&tmp, &file);
      recursive_visit(&tmp, tab + 1);
      cpathCloseDir(&tmp);
    }
  }
}

void print_dir(cf_file_t *file, void *udata) { puts(file->name); }

void recurse(tinydir_dir *dir, int tab) {
  tinydir_file file;
  while (dir->has_next) {
    if (tinydir_readfile(dir, &file) == -1) {
      perror("Error getting file");
      tinydir_close(dir);
      return;
    }

    puts(file.name);
    if (file.is_dir && _tinydir_strcmp(file.name, ".") &&
        _tinydir_strcmp(file.name, "..")) {
      tinydir_dir tmp;
      tinydir_open(&tmp, file.path);
      recurse(&tmp, tab + 1);
      tinydir_close(&tmp);
    }

    if (tinydir_next(dir) == -1) {
      perror("Error getting file");
      tinydir_close(dir);
      return;
    }
  }
}

void emplace(cpath_dir *dir) {
  cpath_file file;
  int tab = 0;

  while (dir != NULL) {
    while (cpathGetNextFile(dir, &file)) {
      for (int i = 0; i < tab; i++)
        putchar('\t');
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

int main(int argc, char *argv[]) {
  OBS_SETUP("CPath", argc, argv);

  // we have to generate the files
  if (has_benchmarks) {
    cpath path = cpathFromUtf8("tmp");
    cpathMkdir(&path);
    for (cpath_char_t i = 1; i < 10; i++) {
      cpathAppendSprintf(&path, "/a%d", i);
      cpathMkdir(&path);
      for (cpath_char_t j = 1; j < 100; j++) {
        cpathAppendSprintf(&path, "/b%d", j);
        cpathMkdir(&path);
        for (cpath_char_t k = 1; k < 50; k++) {
          cpathAppendSprintf(&path, "/%d.tmp", k);
          FILE *f = cpathOpen(&path, CPATH_STR("w"));
          fclose(f);
          cpathUpDir(&path);
        }
        cpathUpDir(&path);
      }
      cpathUpDir(&path);
    }
  }

  OBS_BENCHMARK("Stack CPath", 100, {
    cpath_dir dir;
    cpath path;
    cpathFromStr(&path, "tmp");
    cpathOpenDir(&dir, &path);
    emplace(&dir);
  })

  OBS_BENCHMARK("Recursive CPath", 100, {
    cpath_dir dir;
    cpath path;
    cpathFromStr(&path, "tmp");
    cpathOpenDir(&dir, &path);
    recursive_visit(&dir, 0);
  })

  OBS_BENCHMARK("Recursive Cute Files", 100,
                { cf_traverse("tmp", print_dir, NULL); })

  OBS_BENCHMARK("Recursive TinyDir", 100, {
    tinydir_dir dir;
    if (tinydir_open(&dir, "tmp") == -1) {
      perror("Error opening file");
      tinydir_close(&dir);
      return 0;
    }
    recurse(&dir, 0);
    tinydir_close(&dir);
  })

// Presuming you have tree installed
#if !defined _MSC_VER && !defined __MINGW32__
  OBS_BENCHMARK_SYS("Tree", 100, "tree tmp");
  OBS_BENCHMARK_SYS("Find", 100, "find tmp");
  OBS_BENCHMARK_SYS("Python", 100, "python3 ./benchmark_recursive.py ./tmp")
#endif

  OBS_TEST_GROUP("Path", {
    ;
    OBS_TEST("Empty Path", {
      cpath utf8 = cpathFromUtf8("");
      obs_test_path_eq_string(utf8, ".");
      cpath raw;
      obs_test_true(cpathFromStr(&raw, CPATH_STR("")));
      obs_test_path_eq_string(raw, ".");
      obs_test_path_eq(raw, utf8);
    })

    OBS_TEST("Non empty path", {
      cpath utf8 = cpathFromUtf8("Cat");
      obs_test_path_eq_string(utf8, "Cat");
      cpath raw;
      obs_test_true(cpathFromStr(&raw, CPATH_STR("Cat")));
      obs_test_path_eq_string(raw, "Cat");
      obs_test_path_eq(raw, utf8);
    })

    OBS_TEST("Convert Backslashes", {
      cpath utf8 = cpathFromUtf8("/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(utf8, "/a/b/c/d");
      cpath path;
      cpathFromStr(&path, "/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(path, "/a/b/c/d");
    })

    OBS_TEST("Convert Backslashes on concat", {
      cpath utf8 = cpathFromUtf8("/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(utf8, "/a/b/c/d");
      obs_test_true(CPATH_CONCAT_LIT(&utf8, "\\e/\\f/g\\"));
      obs_test_path_eq_string(utf8, "/a/b/c/d/e/f/g");
      cpath path;
      cpathFromStr(&path, "/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(path, "/a/b/c/d");
      obs_test_path_eq_string(path, "/a/b/c/d");
      obs_test_true(CPATH_CONCAT_LIT(&path, "\\e/\\f/g\\"));
      obs_test_path_eq_string(path, "/a/b/c/d/e/f/g");
    })

    OBS_TEST("Path copy", {
      cpath a = cpathFromUtf8("Cat");
      cpath b;
      cpathCopy(&b, &a);
      obs_test_path_eq(a, b);
      obs_test_path_eq_string(a, "Cat");
      obs_test_true(CPATH_CONCAT_LIT(&a, "Other"));
      obs_test_path_neq(a, b);
      obs_test_path_eq_string(a, "Cat/Other");
    })

    OBS_TEST("Paths concat", {
      cpath base = cpathFromUtf8("A");
      cpath copy;
      cpath copy2;
      cpath_str other = CPATH_STR("B");
      cpathCopy(&copy, &base);
      cpathCopy(&copy2, &base);
      obs_test_true(CPATH_CONCAT_LIT(&base, "B"));
      obs_test_path_neq(base, copy);
      obs_test_path_neq(base, copy2);
      obs_test_path_eq_string(base, "A/B");
      obs_test_path_eq(copy, copy2);

      obs_test_true(cpathConcatStr(&copy, other));
      obs_test_path_eq(base, copy);
      obs_test_path_neq(base, copy2);
      obs_test_path_eq_string(copy, "A/B");
      obs_test_path_neq(copy, copy2);

      obs_test_true(cpathConcatStrn(&copy2, other, 1));
      obs_test_path_eq(base, copy);
      obs_test_path_eq(base, copy2);
      obs_test_path_eq_string(copy2, "A/B");
      obs_test_path_eq(copy, copy2);
    })

    OBS_TEST("Fake canonicalise '/'", {
      cpath a = cpathFromUtf8("/..");
      obs_test_false(cpathCanonicaliseNoSysCall(&a, &a));
      a = cpathFromUtf8("/././../");
      obs_test_false(cpathCanonicaliseNoSysCall(&a, &a));
      a = cpathFromUtf8("/././");
      obs_test_true(cpathCanonicaliseNoSysCall(&a, &a));
      obs_test_path_eq_string(a, "/");
    })

    OBS_TEST("Fake canonicalise '.'", {
      cpath a = cpathFromUtf8("A/./");
      obs_test_true(cpathCanonicaliseNoSysCall(&a, &a));
      obs_test_path_eq_string(a, "A");
      a = cpathFromUtf8("././././");
      obs_test_true(cpathCanonicaliseNoSysCall(&a, &a));
      obs_test_path_eq_string(a, ".");
    })

    OBS_TEST("Fake canonicalise '..'", {
      cpath a = cpathFromUtf8("A/../");
      obs_test_true(cpathCanonicaliseNoSysCall(&a, &a));
      obs_test_path_eq_string(a, ".");
      cpath b = cpathFromUtf8("./../");
      obs_test_false(cpathCanonicaliseNoSysCall(&b, &b));
    })

    OBS_TEST("Fake canonicalise complex", {
      cpath a = cpathFromUtf8("C:/A/../B/../C/../D/./././");
      obs_test_true(cpathCanonicaliseNoSysCall(&a, &a));
      obs_test_path_eq_string(a, "C:/D");
    })

    OBS_TEST("Canonlicalise", {
      cpath a = cpathFromUtf8(".");
      obs_test_true(cpathCanonicalise(&a, &a));
      obs_test_path_eq(a, cpathGetCwd());
    })

    OBS_TEST("Paths exist", {
      cpath dir = cpathFromUtf8("A");
      obs_test_true(cpathExists(&dir));
      dir = cpathFromUtf8("A/B");
      obs_test_true(cpathExists(&dir));

      cpath invalid = cpathFromUtf8("Cat");
      obs_test_false(cpathExists(&invalid));
      CPATH_CONCAT_LIT(&invalid, "../A");
      obs_test_path_eq_string(invalid, "Cat/../A");
      obs_test_true(cpathCanonicaliseNoSysCall(&invalid, &invalid));
      obs_test_path_eq_string(invalid, "A");
      obs_test_true(cpathExists(&invalid));
    })

    OBS_TEST("cpathUpDir", {
      cpath dir = cpathFromUtf8("/A/B/C/D");
      obs_test_true(cpathUpDir(&dir));
      obs_test_path_eq_string(dir, "/A/B/C");
      obs_test_true(cpathUpDir(&dir));
      obs_test_path_eq_string(dir, "/A/B");
    })

    OBS_TEST("CPath Iterator from 0", {
      cpath dir = cpathFromUtf8("/A/B/C/D/E.c");
      const cpath_char_t *it = NULL;
      int index = 0;
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "");
      obs_test_str_eq(dir.buf, "");
      obs_test_str_eq(dir.buf + 1, "A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "A");
      obs_test_str_eq(dir.buf, "/A");
      obs_test_str_eq(dir.buf + 3, "B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "B");
      obs_test_str_eq(dir.buf, "/A/B");
      obs_test_str_eq(dir.buf + 5, "C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "C");
      obs_test_str_eq(dir.buf, "/A/B/C");
      obs_test_str_eq(dir.buf + 7, "D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "D");
      obs_test_str_eq(dir.buf, "/A/B/C/D");
      obs_test_str_eq(dir.buf + 9, "E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "E.c");
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      obs_test_str_eq(dir.buf + 12, "");
      it = cpathItRef(&dir, &index);
      obs_test_null(it);
      obs_test_path_eq_string(dir, "/A/B/C/D/E.c");
    })

    OBS_TEST("CPath Iterator from 0 restoring each time", {
      cpath dir = cpathFromUtf8("/A/B/C/D/E.c");
      const cpath_char_t *it = NULL;
      int index = 0;
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "");
      obs_test_str_eq(dir.buf, "");
      obs_test_str_eq(dir.buf + 1, "A/B/C/D/E.c");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "A");
      obs_test_str_eq(dir.buf, "/A");
      obs_test_str_eq(dir.buf + 3, "B/C/D/E.c");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "B");
      obs_test_str_eq(dir.buf, "/A/B");
      obs_test_str_eq(dir.buf + 5, "C/D/E.c");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "C");
      obs_test_str_eq(dir.buf, "/A/B/C");
      obs_test_str_eq(dir.buf + 7, "D/E.c");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "D");
      obs_test_str_eq(dir.buf, "/A/B/C/D");
      obs_test_str_eq(dir.buf + 9, "E.c");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_not_null(it);
      obs_test_str_eq(it, "E.c");
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      obs_test_str_eq(dir.buf + 12, "");
      cpathItRefRestore(&dir, &index);
      obs_test_str_eq(dir.buf, "/A/B/C/D/E.c");
      it = cpathItRef(&dir, &index);
      obs_test_null(it);
      obs_test_path_eq_string(dir, "/A/B/C/D/E.c");
    })
  })

  OBS_REPORT
  return tests_failed;
}
