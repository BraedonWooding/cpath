// This file should be run with -DCPATH_UNICODE on and off
// This just tests that all \ get convert to /
#define CPATH_FORCE_CONVERSION_SYSTEM

#include "../cpath.h"

#define OBS_STRCMP cpath_str_compare

#include "obsidian.h"
#include "obsidian_extras.h"

#if defined _MSC_VER || defined __mingw32__
#define PATH_EXAMPLE "\\"
#else
#define PATH_EXAMPLE "/"
#endif

int main(int argc, char *argv[]) {
  OBS_SETUP("CPath", argc, argv);

  OBS_TEST_GROUP("Path", {
    ;
    OBS_TEST("Convert Backslashes", {
      const cpath_char_t *res =
          PATH_EXAMPLE "a" PATH_EXAMPLE "b" PATH_EXAMPLE "c" PATH_EXAMPLE "d";

      cpath utf8 = cpathFromUtf8("/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(utf8, res);
      cpath path;
      cpathFromStr(&path, "/a\\b/c\\\\d/\\");
      obs_test_path_eq_string(path, res);
    })

    OBS_TEST("Convert Backslashes with concat", {
      const cpath_char_t *res =
          PATH_EXAMPLE "a" PATH_EXAMPLE "b" PATH_EXAMPLE "c" PATH_EXAMPLE
                       "d" PATH_EXAMPLE "e" PATH_EXAMPLE "f" PATH_EXAMPLE "g";

      cpath utf8 = cpathFromUtf8("/a\\b/c\\\\d/\\");
      obs_test_true(CPATH_CONCAT_LIT(&utf8, "\\e/\\f/g\\"));
      obs_test_path_eq_string(utf8, res);
      cpath path;
      cpathFromStr(&path, "/a\\b/c\\\\d/\\");
      obs_test_true(CPATH_CONCAT_LIT(&path, "\\e/\\f/g\\"));
      obs_test_path_eq_string(path, res);
    })
  })

  OBS_REPORT
  return tests_failed;
}
