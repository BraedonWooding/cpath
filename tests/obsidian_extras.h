#ifndef OBSIDIAN_EXTRAS_H
#define OBSIDIAN_EXTRAS_H

#define obs_test_path_eq(a, b)                                                 \
  obs_test_str_eq(a.buf, b.buf);                                               \
  obs_test_eq(size_t, a.len, b.len);

#define obs_test_path_neq(a, b)                                                \
  obs_test_str_neq(a.buf, b.buf);                                              \
  obs_test_neq(size_t, a.len, b.len);

#define obs_test_path_eq_string(a, str)                                        \
  obs_test_str_eq(a.buf, CPATH_STR(str));                                      \
  obs_test_eq(size_t, a.len, cpath_str_length(CPATH_STR(str)));

#define obs_test_path_neq_string(a, str)                                       \
  obs_test_str_neq(a.buf, CPATH_STR(str));                                     \
  obs_test_neq(size_t, a.len, cpath_str_length(CPATH_STR(str)));

#define obs_test_match_dir(file, exp_path, exp_name)                           \
  obs_test_path_eq_string(file.path, exp_path);                                \
  obs_test_str_eq(file.name, exp_name);                                        \
  obs_test_false(file.isReg);                                                  \
  obs_test_false(file.isSym);                                                  \
  obs_test_true(file.isDir);

#define obs_test_match_file(file, exp_path, exp_name)                          \
  obs_test_path_eq_string(file.path, exp_path);                                \
  obs_test_str_eq(file.name, exp_name);                                        \
  obs_test_true(file.isReg);                                                   \
  obs_test_false(file.isSym);                                                  \
  obs_test_false(file.isDir);

#define obs_test_match_link(file, path, exp_name)                              \
  obs_test_path_eq_string(file.path, exp_path);                                \
  obs_test_str_eq(file.name, exp_name);                                        \
  obs_test_false(file.isReg);                                                  \
  obs_test_true(file.isSym);                                                   \
  obs_test_false(file.isDir);

// NOTE: we can't check path for this
//       won't really matter since path
//       is getting deprecated anyways
#define obs_test_dot(dir, file)                                                \
  if (cpath_str_compare(file.name, ".") == 0) {                                \
    obs_test_str_eq(file.name, ".");                                           \
    obs_test_false(file.isReg);                                                \
    obs_test_false(file.isSym);                                                \
    obs_test_true(file.isDir);                                                 \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
  }                                                                            \
                                                                               \
  if (cpath_str_compare(file.name, "..") == 0) {                               \
    obs_test_str_eq(file.name, "..");                                          \
    obs_test_false(file.isReg);                                                \
    obs_test_false(file.isSym);                                                \
    obs_test_true(file.isDir);                                                 \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
  }                                                                            \
                                                                               \
  if (cpath_str_compare(file.name, ".") == 0) {                                \
    obs_test_str_eq(file.name, ".");                                           \
    obs_test_false(file.isReg);                                                \
    obs_test_false(file.isSym);                                                \
    obs_test_true(file.isDir);                                                 \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
  }

#define obs_test_next_file(dir, file, dir_check, is_dir_code, is_file_code)    \
  obs_test_true(cpathGetNextFile(&dir, &file));                                \
  obs_test_dot(dir, file);                                                     \
  if (file.isDir) {                                                            \
    dir_check;                                                                 \
    obs_test_true(cpathOpenSubFileEmplace(&dir, &file, 1));                    \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
    obs_test_dot(dir, file);                                                   \
    is_dir_code;                                                               \
    obs_test_true(cpathRevertEmplaceCopy(&dir));                               \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
    obs_test_dot(dir, file);                                                   \
    is_file_code;                                                              \
  } else {                                                                     \
    is_file_code;                                                              \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
    dir_check;                                                                 \
    obs_test_true(cpathOpenSubFileEmplace(&dir, &file, 1));                    \
    obs_test_true(cpathGetNextFile(&dir, &file));                              \
    obs_test_dot(dir, file);                                                   \
    is_dir_code;                                                               \
  }

#endif
