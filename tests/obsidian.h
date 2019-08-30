/* 
    MIT License

    Copyright (c) 2018 Braedon Wooding

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef OBSIDIAN_H
#define OBSIDIAN_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef OBSIDIAN_NO_COLOURS
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#else
#define RED   ""
#define GRN   ""
#define YEL   ""
#define BLU   ""
#define MAG   ""
#define CYN   ""
#define WHT   ""
#define RESET ""
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) #x
#endif

#ifndef OBS_STRCMP
#define OBS_STRCMP strcmp
#endif

#define OBS_SETUP(name) {\
    int successes = 0; \
    int failures = 0; \
    int num_groups = 0; \
    int num_tests = 0; \
    int groups_passed = 0; \
    bool success = false; \
    int old_successes = 0; \
    int old_failures = 0; \
    int old_tests = 0; \
    int tests_in_group = 0; \
    int successes_in_group = 0; \
    int failures_in_group = 0; \
    int num_of_asserts = 0; \
    int old_num_of_asserts = 0; \
    char *cur_group = ""; \
    char *log_file_name = argc < 2 ? name ".log" : argv[1]; \
    FILE *log = fopen(log_file_name, "w"); \
    if (log == NULL) perror(log_file_name);

#define OBS_TEST_GROUP(group_name, group) \
    num_groups++; \
    old_successes = successes; \
    old_failures = failures; \
    old_tests = num_tests; \
    old_num_of_asserts = num_of_asserts; \
    cur_group = group_name; \
    printf(BLU "Running" RESET " " group_name " "); \
    group \
    tests_in_group = num_tests - old_tests; \
    successes_in_group = successes - old_successes; \
    failures_in_group = failures - old_failures; \
    if (failures_in_group == 0) { \
        groups_passed++; \
        printf(GRN " successful" RESET " (%d tests passed running %d asserts) \n", tests_in_group, num_of_asserts - old_num_of_asserts); \
    } else { \
        printf(RED " failed" RESET " (%d/%d tests failed) \n", failures_in_group, tests_in_group); \
    }

#define OBS_TEST(name, group) \
    do { \
        __label__ end; \
        num_tests++; \
        success = true; \
        group \
        end: \
        if (success) { \
            successes++; \
            printf(GRN "." RESET); \
        } else { \
            failures++; \
            printf(RED "." RESET); \
        } \
    } while(0);

#define obs_get_fmt(x, y) \
    _Generic((x), \
        long: #x ": %ld, " #y ": %ld", \
        long long: #x ": %lld, " #y ": %lld", \
        size_t: #x ": %zu, " #y ": %zu", \
        unsigned long long: #x ": %ull, " #y ": %ull", \
        unsigned int: #x ": %u, " #y ": %u", \
        int: #x ": %d, " #y ": %d", \
        float: #x ": %f, " #y ": %f", \
        double: #x ": %lf, " #y ": %lf", \
        long double: "%llf, " #y ": %llf", \
        char: #x ": %c, " #y ": %c", \
        char *: #x ": %s, " #y ": %s", \
        bool: #x ": %d, " #y ": %d", \
        default: #x ": %p, " #y ": %p" \
    )

#define obs_test(cond, fmt, args...) \
    do { \
        num_of_asserts++; \
        if (!(cond)) { \
            success = false; \
            fprintf(log, GRN __FILE__":%d" RESET RED " TEST FAILED: " RESET "%s""\n", __LINE__, cur_group); \
            fprintf(log, "Assert is " #cond "\n"); \
            fprintf(log, "Args are: "); \
            fprintf(log, fmt, args); \
            fprintf(log, "\n\n"); \
            goto end; \
        } \
    } while(0);

#define obs_test_binop(x, op, y) obs_test((x op y), obs_get_fmt(x, y), x, (typeof(x))(y));
#define obs_test_eq(x, y) obs_test_binop(x, ==, y);
#define obs_test_neq(x, y) obs_test_binop(x, !=, y);
#define obs_test_lt(x, y) obs_test_binop(x, <, y);
#define obs_test_gt(x, y) obs_test_binop(x, >, y);
#define obs_test_lte(x, y) obs_test_binop(x, <=, y);
#define obs_test_gte(x, y) obs_test_binop(x, >=, y);
#define obs_test_strcmp(x, y) obs_test(OBS_STRCMP(x, y) == 0, #x ": %s, " #y ": %s", x, y);
#define obs_test_strcmp_false(x, y) obs_test(OBS_STRCMP(x, y) != 0, #x ": %s, " #y ": %s", x, y);
#define obs_test_not_null(x) obs_test(x != NULL, #x ": %p", x);
#define obs_test_null(x) obs_test(x == NULL, #x ": %p", x);
#define obs_test_true(x) obs_test_eq(x, true);
#define obs_test_false(x) obs_test_eq(x, false);

// Present results and exits
#define OBS_REPORT \
    fclose(log); \
    if (successes == num_tests) { \
        remove(log_file_name); \
    } else { \
        int c; \
        log = fopen(log_file_name, "r"); \
        while ((c = fgetc(log)) != EOF) { \
            putchar(c); \
        } \
        fclose(log); \
    } \
    return successes != num_tests; \
}

#endif /* OBSIDIAN_H */
