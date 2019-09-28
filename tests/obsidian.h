/*
    MIT License

    Copyright (c) 2018 Braedon Wooding

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

/*
    How to use:
    - Include obsidian.h
    - Define a main function (with argc/argv)
    - OBS_SETUP("Name of App", argc, argv); to parse arguments and the such
        - NOTE: By default it will strip args before
                a -- argument.  i.e. ./tests --benchmarks "My Test" Other -- 2
                will just become `2`
        - You can turn off above by definining OBS_DONT_STRIP_ARGS
    - Define your groups via OBS_TEST_GROUP("Name", { contents... })
        - Inside each group contents you can put any code you want
          To define a test just use OBS_TEST("Name", { contents... })
        - Inside each OBS_TEST's contents you can put multiple obs_test's
          i.e. obs_test_eq(int, a, b);
        There are the following tests:
        - obs_test(cond, args...)
          i.e. obs_test(a > 0, "a (%d) is positive", a);
          You should always provide atleast a string for the printf and how
          ever many args you want after that.
        - obs_test_binop(type, x, op, y)
          i.e. obs_test_binop(int, x, ==, !=, y);
          > Prints out both arguments evaluated values in a nice way
          NOTE: doesn't store either x or y and will evaluate them twice each
        - obs_test_eq(type, x, y) equivalent to obs_test_binop(type, x, =, y)
        - obs_test_neq(type, x, y) equivalent to obs_test_binop(type, x, !=, y)
        - obs_test_lt(type, x, y) equivalent to obs_test_binop(type, x, <, y)
        - obs_test_gt(type, x, y) equivalent to obs_test_binop(type, x, >, y)
        - obs_test_lte(type, x, y) equivalent to obs_test_binop(type, x, <=, y)
        - obs_test_gte(type, x, y) equivalent to obs_test_binop(type, x, >=, y)
        - obs_test_str_eq(x, y)
            Does a string compare of the two arguments and errors if they aren't
            equivalent.
            NOTE: You can override the strcmp function by defining OBS_STRCMP
        - obs_test_str_neq(x, y) opposite of above.
        - obs_test_mem_eq(type, x, y)
            Does a byte comparison, note x and y should be `(type *)`
            i.e. pass them as pointers, this prevents us having to do any voodoo
            and acts like how you would want a memcmp to work.
            NOTE: You can override the memcmp function by defining OBS_MEMCMP
        - obs_test_mem_neq(type, x, y)
            opposite of above
        - obs_test_null(x) => obs_test_eq(void*, x, NULL) with nicer output
        - obs_test_not_null(x) => obs_test_neq(void*, x, NULL) with nicer output
        - obs_test_true(x) => obs_test_eq(bool, x, true) with nicer output
        - obs_test_false(x) => obs_test_eq(bool, x, false) with nicer output

        NOTE: By less output I mean I don't output the constants true, false
              or NULL and often give better assertion outputs
    - At the end you just add OBS_REPORT
    - You can also override the formats by defining OBS_EXTRA_FMT
      where you can define an if else chain (or whatever you want) to convert
      the string representation of the given type to an format output in terms
      of the two arguments from binop and it's children.
        - Note: the type has all spaces removed to make pointers easier to check
        - There is also a max type length of 256 you can modify this if you want
          but it really should be big enough for any typename.
    - If you want to suppress output of a certain stream or file you can use the
      redirect handlers OBS_REDIRECT_DEVNULL(id, out) and OBS_RESTORE(id, out)
      i.e. we automatically suppress stdout for all tests (not stderr though)
           by just adding OBS_REDIRECT_DEVNULL(stdout, stdout) before all tests
           run and then restoring it after via OBS_RESTORE(stdout, stdout)
      the id is so you can redirect multiple in most cases it'll just match
      the file name like stdout.
      - If you want to disable all redirections just define OBS_NO_REDIRECT
    - You can remove colour input via the #define OBS_NO_COLOURS
    - You can override the raised signal via #define OBS_ERROR_SIGNAL
      and OBS_ERROR_SIGNAL_NAME
    - You can disable benchmarking with OBS_NO_BENCHMARKS and
      OBS_NO_BENCHMARKS_CHILD for just disabling child process benchmarks
    - You can override the time functions (defaults to using cbench) by
      OBS_GET_TIME, OBS_GET_WALL_TIME, OBS_GET_CHILDREN_TIME and obsTime
      - The children time is only required of OBS_NO_BENCHMARKS_CHILD is not
        defined.
      - obsTime should be a struct containing 2 doubles userTime and systemTime.
        spelt exactly that way.  It can contain extra stuff but should aim for
        just those.
    - You can disable all output (the files are still created) via OBS_NO_PRINT

    Arguments that obsidian will handle:
    '--benchmarks' or '-b' =>
        Followed by a series of space delimited benchmark names remember to
        quote any tests with spaces.  It'll only run these benchmarks.
    '--tests' or '-t' => Same as benchmarks but for tests
    '--test-groups' or '-g' => Same as tests/benchmarks but for test groups
    '--raise-on-error' or '-e' =>
        Raise a signal on the first error allowing you to debug the test
        using a standard debugger.  The signal is #defined.
    '--version' or '-v' => print out version information

    Debugging Tips:
    - Find all stack frames ('bt')
    - Pick the most appropriate one (typically you want main) via 'f' i.e. 'f 3'
    - Then you can use your standard print functions :)
*/

#ifndef OBSIDIAN_H
#define OBSIDIAN_H

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OBS_MAJOR_V "1"
#define OBS_MINOR_V "1"
#define OBS_PATCH_V "a"

#define OBS_VERSION OBS_MAJOR_V "." OBS_MINOR_V "." OBS_PATCH_V

#ifndef OBS_NO_COLOURS
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"
#else
#define RED ""
#define GRN ""
#define YEL ""
#define BLU ""
#define MAG ""
#define CYN ""
#define WHT ""
#define RESET ""
#endif

#ifdef _MSC_VER
#define _OBSIDIAN_FUNC_ static __inline
#elif !defined __STDC_VERSION__ || __STDC_VERSION__ < 199901L
#define _OBSIDIAN_FUNC_ static __inline__
#else
#define _OBSIDIAN_FUNC_ static inline
#endif

#if (defined OBS_ERROR_SIGNAL && !defined OBS_ERROR_SIGNAL_NAME) ||            \
    (defined OBS_ERROR_SIGNAL_NAME && !defined OBS_ERROR_SIGNAL)
#error "Must provide OBS_ERROR_SIGNAL and OBS_ERROR_SIGNAL_NAME"
#endif

#ifndef OBS_ERROR_SIGNAL
#define OBS_ERROR_SIGNAL SIGABRT
#define OBS_ERROR_SIGNAL_NAME "SIGABRT"
#endif

#ifndef OBS_STRCMP
#define OBS_STRCMP strcmp
#endif

#ifndef OBS_MEMCMP
#define OBS_MEMCMP memcmp
#endif

#if !defined OBS_NO_BENCHMARKS
// Note you can provide custom functions as shown below
#if (defined OBS_GET_TIME && !defined OBS_GET_WALL_TIME) ||                    \
    (defined OBS_GET_WALL_TIME && !defined OBS_GET_TIME)
#error "Must provide both OBS_GET_TIME and OBS_GET_WALL_TIME"
#endif

#if !defined OBS_NO_BENCHMARKS_CHILD
#if (defined OBS_GET_CHILDREN_TIME && !defined OBS_GET_WALL_TIME) ||           \
    (defined OBS_GET_CHILDREN_TIME && !defined OBS_GET_TIME) ||                \
    (defined OBS_GET_TIME && !defined OBS_GET_CHILDREN_TIME)
#error "Must provide OBS_GET_CHILDREN_TIME"
#endif
#endif

#if !defined OBS_GET_TIME && !defined OBS_GET_WALL_TIME

#if defined CBENCH_VERSION
#if (defined _MSC_VER || defined __MINGW32__)
#define OBS_NO_BENCHMARKS_CHILD
#endif

#define OBS_GET_TIME cbenchGetTime
#define OBS_GET_WALL_TIME cbenchGetWallTime
#define OBS_GET_CHILDREN_TIME cbenchGetChildrenTime
#define obsTime cbenchTime
#else
#define OBS_NO_CBENCH
#endif

#else
#if (!defined obsTime)
#error "Must also provide the time struct"
#endif
#endif
#endif

#ifndef OBS_MAX_TYPE_SIZE
#define OBS_MAX_TYPE_SIZE (256)
#endif

#if !defined OBS_NO_REDIRECT
#if defined _MSC_VER || defined __MINGW32__
#include <io.h>
#define obs_devnull ("NUL")
#define OBS_DUP(fd) _dup(fd)
#define OBS_DUP2(fd, newfd) _dup2(fd, newfd)
#else
#include <unistd.h>
#define obs_devnull ("/dev/null")
#define OBS_DUP(fd) dup(fd)
#define OBS_DUP2(fd, newfd) dup2(fd, newfd)
#endif

#define OBS_REDIRECT(id, out, to, mode)                                        \
  fflush(out);                                                                 \
  int id##_stdoutBackupFd = OBS_DUP(fileno(out));                              \
  FILE *id##_toOut = fopen(to, mode);                                          \
  OBS_DUP2(fileno(id##_toOut), fileno(out));

#define OBS_REDIRECT_DEVNULL(id, out)                                          \
  fflush(out);                                                                 \
  int id##_stdoutBackupFd = OBS_DUP(fileno(out));                              \
  FILE *id##_toOut = fopen(obs_devnull, "w");                                  \
  OBS_DUP2(fileno(id##_toOut), fileno(out));

#define OBS_RESTORE(id, out)                                                   \
  fflush(out);                                                                 \
  fclose(id##_toOut);                                                          \
  OBS_DUP2(id##_stdoutBackupFd, fileno(out));                                  \
  close(id##_stdoutBackupFd);
#else
#define OBS_REDIRECT(id, out, to, mode) ;
#define OBS_REDIRECT_DEVNULL(id, out) ;
#define OBS_RESTORE(id, out) ;
#endif

#define OBS_SETUP(name, argc, argv)                                            \
  int tests_failed = 0;                                                        \
  bool has_benchmarks = false;                                                 \
  bool has_tests = false;                                                      \
  bool has_groups = false;                                                     \
  {                                                                            \
    int successes = 0;                                                         \
    int failures = 0;                                                          \
    int num_groups = 0;                                                        \
    int benchmarks_run = 0;                                                    \
    int num_tests = 0;                                                         \
    int groups_passed = 0;                                                     \
    bool success = true;                                                       \
    int old_successes = 0;                                                     \
    int old_failures = 0;                                                      \
    int old_tests = 0;                                                         \
    int tests_in_group = 0;                                                    \
    int successes_in_group = 0;                                                \
    int failures_in_group = 0;                                                 \
    int num_of_asserts = 0;                                                    \
    int old_num_of_asserts = 0;                                                \
    bool raise_on_err = false;                                                 \
    char **test_start = NULL;                                                  \
    char **benchmark_start = NULL;                                             \
    char **group_start = NULL;                                                 \
    int num_tests_filter = 0;                                                  \
    int num_benchmarks_filter = 0;                                             \
    int num_group_filter = 0;                                                  \
    char *cur_group = "";                                                      \
    char *cur_test = "";                                                       \
    char *log_file_name = name ".log";                                         \
    char *err_log_file_name = name "_err.log";                                 \
    char *benchmarks_log_file_name = name "_benchmark.log";                    \
    FILE *log = fopen(log_file_name, "w+");                                    \
    if (log == NULL) {                                                         \
      perror(log_file_name);                                                   \
    }                                                                          \
    FILE *err_log = fopen(err_log_file_name, "w+");                            \
    if (err_log == NULL) {                                                     \
      perror(err_log_file_name);                                               \
    }                                                                          \
    FILE *benchmark_log = fopen(benchmarks_log_file_name, "w+");               \
    if (benchmark_log == NULL) {                                               \
      perror(benchmarks_log_file_name);                                        \
    }                                                                          \
    obs_parse_args_(log, &argc, &argv, &raise_on_err, &test_start,             \
                    &num_tests_filter, &benchmark_start,                       \
                    &num_benchmarks_filter, &group_start, &num_group_filter);  \
    has_groups = group_start == NULL || num_group_filter > 0;                  \
    has_benchmarks = benchmark_start == NULL || num_benchmarks_filter > 0;     \
    has_tests = test_start == NULL || num_tests_filter > 0;                    \
    OBS_REDIRECT_DEVNULL(stdout, stdout);

#define OBS_TEST_GROUP(group_name, ...)                                        \
  if (obs_can_run_(group_name, group_start, num_group_filter)) {               \
    num_groups++;                                                              \
    old_successes = successes;                                                 \
    old_failures = failures;                                                   \
    old_tests = num_tests;                                                     \
    old_num_of_asserts = num_of_asserts;                                       \
    cur_group = group_name;                                                    \
    fprintf(log, BLU "Running" RESET " " group_name " ");                      \
    __VA_ARGS__;                                                               \
    tests_in_group = num_tests - old_tests;                                    \
    successes_in_group = successes - old_successes;                            \
    failures_in_group = failures - old_failures;                               \
    if (failures_in_group == 0) {                                              \
      groups_passed++;                                                         \
      fprintf(log,                                                             \
              GRN " successful" RESET                                          \
                  " (%d tests passed running %d asserts) \n",                  \
              tests_in_group, num_of_asserts - old_num_of_asserts);            \
    } else {                                                                   \
      fprintf(log, RED " failed" RESET " (%d/%d tests failed) \n",             \
              failures_in_group, tests_in_group);                              \
    }                                                                          \
  }

#if !defined OBS_NO_BENCHMARKS && !defined OBS_NO_CBENCH
#define OBS_BENCHMARK_CUSTOM(benchmark_name, repetitions, get_time, wall_time, \
                             ...)                                              \
  if (obs_can_run_(benchmark_name, benchmark_start, num_benchmarks_filter)) {  \
    fprintf(benchmark_log, BLU "Benchmarking" RESET " " benchmark_name         \
                               " (" #repetitions " time/s) \n");               \
    double max_sys = -1, min_sys = -1;                                         \
    double max_usr = -1, min_usr = -1;                                         \
    double max_total = -1, min_total = -1;                                     \
    benchmarks_run++;                                                          \
    double avg_sys = 0, avg_usr = 0, avg_total = 0;                            \
    int failed = 0;                                                            \
    int i;                                                                     \
    for (i = 0; i < repetitions; ++i) {                                        \
      double startReal = wall_time();                                          \
      cbenchTime start = get_time();                                           \
      __VA_ARGS__;                                                             \
      cbenchTime end = get_time();                                             \
      double endReal = wall_time();                                            \
      if (start.userTime == -1 || start.systemTime == -1 ||                    \
          end.userTime == -1 || end.systemTime == -1) {                        \
        failed = 1;                                                            \
        break;                                                                 \
      }                                                                        \
      double diff_sys = end.systemTime - start.systemTime;                     \
      double diff_usr = end.userTime - start.userTime;                         \
      double diff_total = endReal - startReal;                                 \
      avg_sys += diff_sys;                                                     \
      avg_usr += diff_usr;                                                     \
      avg_total += diff_total;                                                 \
      if (max_sys == -1 || max_sys < diff_sys)                                 \
        max_sys = diff_sys;                                                    \
      if (min_sys == -1 || min_sys > diff_sys)                                 \
        min_sys = diff_sys;                                                    \
      if (max_usr == -1 || max_usr < diff_usr)                                 \
        max_usr = diff_usr;                                                    \
      if (min_usr == -1 || min_usr > diff_usr)                                 \
        min_usr = diff_usr;                                                    \
      if (max_total == -1 || max_total < diff_total)                           \
        max_total = diff_total;                                                \
      if (min_total == -1 || min_total > diff_total)                           \
        min_total = diff_total;                                                \
    }                                                                          \
    if (failed) {                                                              \
      fprintf(benchmark_log, "Benchmark " RED "failed\n" RESET);               \
    } else {                                                                   \
      avg_sys /= repetitions;                                                  \
      avg_usr /= repetitions;                                                  \
      avg_total /= repetitions;                                                \
      double range_sys = (max_sys - min_sys) / 2;                              \
      double range_usr = (max_usr - min_usr) / 2;                              \
      double range_total = (max_total - min_total) / 2;                        \
      fprintf(benchmark_log,                                                   \
              "| usr (s)        | sys (s)        | wall (s)       |\n");       \
      fprintf(benchmark_log,                                                   \
              "| -------------- | -------------- | -------------- |\n");       \
      if (repetitions == 1) {                                                  \
        fprintf(benchmark_log,                                                 \
                "| %.3lf          | %.3lf          | %.3lf          |\n",      \
                avg_usr, avg_sys, avg_total);                                  \
      } else {                                                                 \
        fprintf(benchmark_log,                                                 \
                "| %.3lf +- %.3lf | %.3lf +- %.3lf | %.3lf +- %.3lf |\n",      \
                avg_usr, range_usr, avg_sys, range_sys, avg_total,             \
                range_total);                                                  \
      }                                                                        \
    }                                                                          \
    fprintf(benchmark_log, "\n");                                              \
  }

#define OBS_BENCHMARK(benchmark_name, repetitions, ...)                        \
  OBS_BENCHMARK_CUSTOM(benchmark_name, repetitions, OBS_GET_TIME,              \
                       OBS_GET_WALL_TIME, __VA_ARGS__);

#ifndef OBS_NO_BENCHMARKS_CHILD
#define OBS_BENCHMARK_CHILD(benchmark_name, repetitions, ...)                  \
  OBS_BENCHMARK_CUSTOM(benchmark_name, repetitions, OBS_GET_CHILDREN_TIME,     \
                       OBS_GET_WALL_TIME, __VA_ARGS__);
#define OBS_BENCHMARK_SYS(benchmark_name, repetitions, command)                \
  OBS_BENCHMARK_CHILD(benchmark_name, repetitions, { system(command); });
#else
#define OBS_BENCHMARK_CHILD(...) ;
#define OBS_BENCHMARK_SYS(...) ;
#endif

#elif defined OBS_NO_CBENCH
#define OBS_BENCHMARK(...) @"No benchmarking library included";
#define OBS_BENCHMARK_SYS(...) @"No benchmarking library included";
#define OBS_BENCHMARK_CHILD(...) @"No benchmarking library included";
#define OBS_BENCHMARK_CUSTOM(...) @"No benchmarking library included";
#else
#define OBS_BENCHMARK(...) ;
#define OBS_BENCHMARK_SYS(...) ;
#define OBS_BENCHMARK_CHILD(...) ;
#define OBS_BENCHMARK_CUSTOM(...) ;
#endif

#define OBS_TEST(name, ...)                                                    \
  if (obs_can_run_(name, test_start, num_tests_filter)) {                      \
    num_tests++;                                                               \
    success = true;                                                            \
    cur_test = name;                                                           \
    __VA_ARGS__;                                                               \
    if (success) {                                                             \
      successes++;                                                             \
      fprintf(log, GRN "." RESET);                                             \
    } else {                                                                   \
      failures++;                                                              \
      fprintf(log, RED "." RESET);                                             \
    }                                                                          \
    success = true;                                                            \
  }

#define obs_test(cond, ...)                                                    \
  do {                                                                         \
    num_of_asserts++;                                                          \
    if (success && !(cond)) {                                                  \
      if (raise_on_err) {                                                      \
        raise(OBS_ERROR_SIGNAL);                                               \
      }                                                                        \
      success = false;                                                         \
      fprintf(err_log,                                                         \
              GRN __FILE__ ":%d" RESET RED " TEST FAILED: " RESET "%s:%s"      \
                           "\n",                                               \
              __LINE__, cur_group, cur_test);                                  \
      fprintf(err_log, "Assert " #cond "\n");                                  \
      fprintf(err_log, __VA_ARGS__);                                           \
      fprintf(err_log, "\n\n");                                                \
    }                                                                          \
  } while (0)

#define obs_err(...)                                                           \
  do {                                                                         \
    if (success) {                                                             \
      if (raise_on_err) {                                                      \
        raise(OBS_ERROR_SIGNAL);                                               \
      }                                                                        \
      success = false;                                                         \
      fprintf(err_log,                                                         \
              GRN __FILE__ ":%d" RESET RED " TEST FAILED: " RESET "%s:%s"      \
                           "\n",                                               \
              __LINE__, cur_group, cur_test);                                  \
      fprintf(err_log, __VA_ARGS__);                                           \
      fprintf(err_log, "\n\n");                                                \
    }                                                                          \
  } while (0)

#define obs_test_binop(type, x, op, y)                                         \
  obs_test(((x)op(y)), obs_get_format_(#type), (type)(x),                      \
           obs_get_neg_op_(#op), (type)(y))
#define obs_test_eq(type, x, y) obs_test_binop(type, x, ==, y)
#define obs_test_neq(type, x, y) obs_test_binop(type, x, !=, y)
#define obs_test_lt(type, x, y) obs_test_binop(type, x, <, y)
#define obs_test_gt(type, x, y) obs_test_binop(type, x, >, y)
#define obs_test_lte(type, x, y) obs_test_binop(type, x, <=, y)
#define obs_test_gte(type, x, y) obs_test_binop(type, x, >=, y)
#define obs_test_str_eq(x, y)                                                  \
  obs_test(OBS_STRCMP(x, y) == 0, obs_get_format_("char*"), (x), "==", (y))
#define obs_test_str_neq(x, y)                                                 \
  obs_test(OBS_STRCMP(x, y) != 0, obs_get_format_("char*"), (x), "!=", (y))
#define obs_test_not_null(x) obs_test((x) != NULL, #x " is NULL")
#define obs_test_null(x) obs_test((x) == NULL, #x " = %p is not NULL", (x))
#define obs_test_true(x) obs_test((x) == true, #x " = %d is not true", (x))
#define obs_test_false(x) obs_test((x) == false, #x " is not false")
#define obs_test_mem_eq(type, x, y)                                            \
  obs_test(OBS_MEMCMP((type *)(x), (type *)(y), sizeof(type)) == 0,            \
           obs_get_format_(#type), *(type *)(x), "!=", *(type *)(y));
#define obs_test_mem_neq(type, x, y)                                           \
  obs_test(OBS_MEMCMP((type *)(x), (type *)(y), sizeof(type)) != 0,            \
           obs_get_format_(#type), *(type *)(x), "==", *(type *)(y));

_OBSIDIAN_FUNC_ const char *obs_get_format_(char *type) {
  // We only support types up to 256 characters in size
  // Strip all spaces so we can support pointers better
  static char buf[OBS_MAX_TYPE_SIZE];
  size_t len = 0;
  while (*type != '\0') {
    if (*type != ' ')
      buf[len++] = *type;
    type++;
  }

#ifdef OBS_EXTRA_FMT
  OBS_EXTRA_FMT
#endif

  if (!strcmp(buf, "int") || !strcmp(buf, "signed") ||
      !strcmp(buf, "signed int")) {
    return "Evaluated: %i %s %i";
  } else if (!strcmp(buf, "long") || !strcmp(buf, "longint") ||
             !strcmp(buf, "signedlong") || !strcmp(buf, "signedlongint")) {
    return "Evaluated: %li %s %li";
  } else if (!strcmp(buf, "longlong") || !strcmp(buf, "longlongint") ||
             !strcmp(buf, "signedlonglong") ||
             !strcmp(buf, "signedlonglongint")) {
    return "Evaluated: %lli %s %lli";
  } else if (!strcmp(buf, "size_t")) {
    return "Evaluated: %zu %s %zu";
  } else if (!strcmp(buf, "ptrdiff_t")) {
    return "Evaluated: %zd %s %zd";
  } else if (!strcmp(buf, "unsigned") || !strcmp(buf, "unsignedint")) {
    return "Evaluated: %u %s %u";
  } else if (!strcmp(buf, "unsignedlong") || !strcmp(buf, "unsignedlongint")) {
    return "Evaluated: %lu %s %lu";
  } else if (!strcmp(buf, "unsignedlonglong") ||
             !strcmp(buf, "unsignedlonglongint")) {
    return "Evaluated: %llu %s %llu";
  } else if (!strcmp(buf, "float")) {
    return "Evaluated: %f %s %f";
  } else if (!strcmp(buf, "double")) {
    return "Evaluated: %lf %s %lf";
  } else if (!strcmp(buf, "longdouble")) {
    return "Evaluated: %Lf %s %Lf";
  } else if (!strcmp(buf, "char") || !strcmp(buf, "signedchar") ||
             !strcmp(buf, "unsignedchar")) {
    return "Evaluated: '%c' %s '%c'";
  } else if (!strcmp(buf, "char*")) {
    return "Evaluated: \"%s\" %s \"%s\"";
  } else if (!strcmp(buf, "void*")) {
    return "Evaluated: %p %s %p";
  } else if (!strcmp(buf, "bool") || !strcmp(buf, "_Bool")) {
    return "Evaluated: %d %s %d";
  } else {
    return "Evaluated: %p %s %p";
  }
}

// Present results and exits
#ifndef OBS_NO_PRINT
#define OBS_REPORT                                                             \
  OBS_RESTORE(stdout, stdout);                                                 \
  int c;                                                                       \
  rewind(log);                                                                 \
  while ((c = fgetc(log)) != EOF) {                                            \
    putchar(c);                                                                \
  }                                                                            \
  fclose(log);                                                                 \
  if (num_groups == 0) {                                                       \
    remove(log_file_name);                                                     \
  } else if (successes == num_tests) {                                         \
    printf("Log Files are at: %s\n", log_file_name);                           \
    fclose(err_log);                                                           \
    remove(err_log_file_name);                                                 \
  } else {                                                                     \
    printf("Log Files are at: %s and %s\n\n==Errors==\n", log_file_name,       \
           err_log_file_name);                                                 \
    rewind(err_log);                                                           \
    while ((c = fgetc(err_log)) != EOF) {                                      \
      putchar(c);                                                              \
    }                                                                          \
    fclose(err_log);                                                           \
  }                                                                            \
  if (benchmarks_run > 0) {                                                    \
    printf("Benchmark log file is at: %s\n\n==Benchmarks==\n",                 \
           benchmarks_log_file_name);                                          \
    rewind(benchmark_log);                                                     \
    while ((c = fgetc(benchmark_log)) != EOF) {                                \
      putchar(c);                                                              \
    }                                                                          \
    fclose(benchmark_log);                                                     \
  } else {                                                                     \
    fclose(benchmark_log);                                                     \
    remove(benchmarks_log_file_name);                                          \
  }                                                                            \
  tests_failed = failures;                                                     \
  }
#else
#define OBS_REPORT                                                             \
  OBS_RESTORE(stdout, stdout);                                                 \
  fclose(log);                                                                 \
  if (num_groups == 0) {                                                       \
    remove(log_file_name);                                                     \
  } else if (successes == num_tests) {                                         \
    fclose(err_log);                                                           \
    remove(err_log_file_name);                                                 \
  } else {                                                                     \
    fclose(err_log);                                                           \
  }                                                                            \
  if (benchmarks_run > 0) {                                                    \
    fclose(benchmark_log);                                                     \
  } else {                                                                     \
    fclose(benchmark_log);                                                     \
    remove(benchmarks_log_file_name);                                          \
  }                                                                            \
  tests_failed = failures;                                                     \
  }
#endif

/*
    Below is internals you don't need to run these.
*/

// Can we run the test/benchmark based on settings
_OBSIDIAN_FUNC_ bool obs_can_run_(char *name, char **start, int len) {
  if (start == NULL) {
    return true;
  }

  int i;
  for (i = 0; i < len; i++) {
    if (!strcmp(name, start[i])) {
      return true;
    }
  }
  return false;
}

_OBSIDIAN_FUNC_ const char *obs_get_neg_op_(const char *op) {
  if (0) {
  }
#ifdef OBS_EXTRA_NEG_OP
  OBS_EXTRA_NEG_OP
#endif
  else if (!strcmp(op, "==")) return "!=";
  else if (!strcmp(op, "!=")) return "==";
  else if (!strcmp(op, ">")) return "<=";
  else if (!strcmp(op, "<")) return ">=";
  else if (!strcmp(op, ">=")) return "<";
  else if (!strcmp(op, "<=")) return ">";
  else return "and";
}

// Parse all the relevant args
_OBSIDIAN_FUNC_ bool obs_parse_args_(FILE *out, int *argc, char ***argv,
                                     bool *raise_on_err, char ***test_start,
                                     int *num_tests, char ***benchmark_start,
                                     int *num_benchmarks,
                                     char ***test_group_start,
                                     int *num_groups) {
  *test_start = NULL;
  *benchmark_start = NULL;
  *raise_on_err = false;
  *num_benchmarks = 0;
  *num_tests = 0;
  int *last_counter = NULL;
  bool custom_settings = false;
  bool version = false;

  int i;
  for (i = 1; i < *argc; i++) {
    if (!strcmp((*argv)[i], "--tests") || !strcmp((*argv)[i], "-t")) {
      if (*test_start != NULL) {
        fprintf(stderr, "Parse Error: you can't have multiple"
                        "'--tests'\n");
        return false;
      } else {
        *test_start = *argv + i + 1;
        last_counter = num_tests;
        custom_settings = true;
      }
    }
#if !defined OBS_NO_BENCHMARKS
    else if (!strcmp((*argv)[i], "--benchmarks") || !strcmp((*argv)[i], "-b")) {
      if (*benchmark_start != NULL) {
        fprintf(stderr, "Parse Error: you can't have multiple"
                        "'--benchmarks'\n");
        return false;
      } else {
        *benchmark_start = *argv + i + 1;
        last_counter = num_benchmarks;
        custom_settings = true;
      }
    }
#endif
    else if (!strcmp((*argv)[i], "--test-groups") ||
             !strcmp((*argv)[i], "-g")) {
      if (*test_group_start != NULL) {
        fprintf(stderr, "Parse Error: you can't have multiple"
                        "'--test-groups'\n");
        return false;
      } else {
        *test_group_start = *argv + i + 1;
        last_counter = num_groups;
        custom_settings = true;
      }
    } else if (!strcmp((*argv)[i], "--raise-on-error") ||
               !strcmp((*argv)[i], "-e")) {
      if (*raise_on_err) {
        fprintf(stderr,
                "Parse Error: you can't have multiple raise on errors\n");
        return false;
      }
      *raise_on_err = true;
      custom_settings = true;
    } else if (!strcmp((*argv)[i], "--version") || !strcmp((*argv)[i], "-v")) {
      version = true;
      custom_settings = true;
    }
#ifndef OBS_DONT_STRIP_ARGS
    else if (!strcmp((*argv)[i], "--")) {
      // redirect this one to the first one
      *argc -= i;
      (*argv)[i] = (*argv)[0];
      *argv = *argv + i;
      break;
    }
#endif
    else {
      if (last_counter != NULL) {
        (*last_counter)++;
      } else {
        fprintf(stderr, "Unexpected argument: %s\n", (*argv)[i]);
        return false;
      }
    }
  }

  if (custom_settings) {
    fprintf(out, "== Settings ==\n");
  }
  if (version) {
    fprintf(out, "Obsidian version v" OBS_VERSION "\n");
#ifdef CBENCH_VERSION
    fprintf(out, "CBench version v" CBENCH_VERSION "\n");
#endif
  }
  if (*raise_on_err) {
    fprintf(out, "- Raising signal ");
    fprintf(out, OBS_ERROR_SIGNAL_NAME);
    fprintf(out, "(%d) on error\n", OBS_ERROR_SIGNAL);
  }
#define OBS_TEST_INFO(start, count, name)                                      \
  if (*start != NULL) {                                                        \
    if (*count > 0) {                                                          \
      fprintf(out, "- Running " name ": %s", (*start)[0]);                     \
      int i;                                                                   \
      for (i = 1; i < *count; i++) {                                           \
        fprintf(out, " %s", (*start)[i]);                                      \
      }                                                                        \
      fprintf(out, "\n");                                                      \
    } else {                                                                   \
      fprintf(out, "- Running no " name "\n");                                 \
    }                                                                          \
  }

  OBS_TEST_INFO(test_start, num_tests, "tests");
  OBS_TEST_INFO(benchmark_start, num_benchmarks, "benchmarks");
  OBS_TEST_INFO(test_group_start, num_groups, "groups");
#undef OBS_TEST_INFO

  if (custom_settings) {
    fprintf(out, "\n");
  }

  return true;
}

#endif /* OBSIDIAN_H */
