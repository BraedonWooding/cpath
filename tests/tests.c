// This file should be run with -DCPATH_UNICODE on and off

#include "../path.h"

#define OBS_STRCMP cpath_str_compare

#include "obsidian.h"

#define obs_test_path_eq(a, b) \
    obs_test_eq(a.len, b.len); \
    obs_test_strcmp(a.buf, b.buf);

#define obs_test_path_neq(a, b) \
    obs_test_neq(a.len, b.len); \
    obs_test_strcmp_false(a.buf, b.buf);

#define obs_test_path_eq_string(a, str) \
    obs_test_eq(a.len, cpath_str_length(CPATH_STR(str))); \
    obs_test_strcmp(a.buf, CPATH_STR(str));

#define obs_test_path_neq_string(a, str) \
    obs_test_neq(a.len, cpath_str_length(CPATH_STR(str))); \
    obs_test_strcmp_false(a.buf, CPATH_STR(str));

int main(int argc, char *argv[]) {
    OBS_SETUP("CPath")

    OBS_TEST_GROUP("Path", {
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

        // @BUG: `/` should be the user directory

        OBS_TEST("Fake canonicalise `.`", {
            cpath a = cpathFromUtf8("A/./");
            obs_test_true(cpathCanonicaliseFake(&a));
            obs_test_path_eq_string(a, "A");
            a = cpathFromUtf8("././././");
            obs_test_true(cpathCanonicaliseFake(&a));
            obs_test_path_eq_string(a, ".");
        })

        OBS_TEST("Fake canonicalise `..`", {
            cpath a = cpathFromUtf8("A/../");
            obs_test_true(cpathCanonicaliseFake(&a));
            obs_test_path_eq_string(a, ".");
            cpath b = cpathFromUtf8("./../");
            obs_test_false(cpathCanonicaliseFake(&b));
        })

        OBS_TEST("Paths exist", {
            cpath dir = cpathFromUtf8("A");
            obs_test_true(cpathExists(&dir));
            dir = cpathFromUtf8("A/B");
            obs_test_true(cpathExists(&dir));

            cpath invalid = cpathFromUtf8("Cat");
            obs_test_false(cpathExists(&invalid));
            CPATH_CONCAT_LIT(&invalid, "../A");
            obs_test_strcmp(invalid.buf, CPATH_STR("Cat/../A"))
            obs_test_true(cpathCanonicaliseFake(&invalid));
            obs_test_strcmp(invalid.buf, CPATH_STR("A"))
            obs_test_true(cpathExists(&invalid));
        })
    })

    OBS_REPORT
}
