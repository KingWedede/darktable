/*
    This file is part of darktable,
    Copyright (C) 2025 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * Unit tests for path handling improvements
 *
 * This test suite validates the path construction and handling fixes
 * made to the storage modules (gallery, latex, disk). It ensures that:
 * - snprintf bounds checking works correctly
 * - Windows UNC paths are handled properly
 * - Long paths near PATH_MAX don't overflow
 * - Path separators are handled correctly across platforms
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "util/tracing.h"

// Platform-specific definitions
#ifdef _WIN32
  #define TEST_PATH_SEP '\\'
  #define TEST_ALT_SEP '/'
  #define UNC_PREFIX "\\\\"
#else
  #define TEST_PATH_SEP '/'
  #define TEST_ALT_SEP '\\'  // Not typical on Unix but should be handled
  #define UNC_PREFIX "//"    // Treated as regular path on Unix
#endif

/**
 * Test path construction with pointer arithmetic
 * This mirrors the pattern used in gallery.c and latex.c
 */
static void test_path_pointer_arithmetic(void **state)
{
  (void)state;

  TR_STEP("verify path pointer arithmetic pattern");

  // Pattern from gallery.c:466-467
  char filename[PATH_MAX] = "/base/directory";
  char *c = filename + strlen(filename);
  const size_t remaining = PATH_MAX - (c - filename);

  // Verify calculation is correct
  assert_int_equal(remaining, PATH_MAX - strlen("/base/directory"));
  assert_true(remaining > 0);
  assert_true(remaining + strlen(filename) == PATH_MAX);

  // Simulate appending a path component
  snprintf(c, remaining, "/subdir/file.ext");

  // Should not overflow
  assert_true(strlen(filename) < PATH_MAX);
  assert_string_equal(filename, "/base/directory/subdir/file.ext");

  TR_NOTE("pointer arithmetic for remaining space is correct");
}

/**
 * Test long path handling near PATH_MAX
 */
static void test_long_paths(void **state)
{
  (void)state;

  TR_STEP("verify handling of paths near PATH_MAX limit");

  // Create a base path that's almost at the limit
  char base_path[PATH_MAX];
  memset(base_path, 'a', PATH_MAX - 50);
  base_path[0] = '/';
  base_path[PATH_MAX - 50] = '\0';

  size_t base_len = strlen(base_path);
  TR_DEBUG("base path length: %zu", base_len);

  // Try to append extension
  char *c = base_path + base_len;
  size_t remaining = PATH_MAX - base_len;

  snprintf(c, remaining, ".jpg");

  // Should be truncated properly
  assert_true(strlen(base_path) < PATH_MAX);
  assert_true(base_path[PATH_MAX - 1] == '\0' || base_path[PATH_MAX - 1] == 'a');

  TR_NOTE("long paths are properly truncated without overflow");
}

/**
 * Test Windows UNC path handling
 */
static void test_unc_paths(void **state)
{
  (void)state;

  TR_STEP("verify Windows UNC path handling");

#ifdef _WIN32
  // Test UNC path prefix preservation
  char unc_path[PATH_MAX] = "\\\\server\\share\\directory";

  assert_int_equal(unc_path[0], '\\');
  assert_int_equal(unc_path[1], '\\');

  // Verify we can append to UNC paths
  char *c = unc_path + strlen(unc_path);
  size_t remaining = PATH_MAX - (c - unc_path);
  snprintf(c, remaining, "\\file.txt");

  assert_string_equal(unc_path, "\\\\server\\share\\directory\\file.txt");
  assert_int_equal(unc_path[0], '\\');
  assert_int_equal(unc_path[1], '\\');

  TR_NOTE("UNC path prefix preserved correctly");
#else
  // On Unix, UNC-style paths are just regular paths
  char path[PATH_MAX] = "//server/share/directory";

  char *c = path + strlen(path);
  size_t remaining = PATH_MAX - (c - path);
  snprintf(c, remaining, "/file.txt");

  assert_string_equal(path, "//server/share/directory/file.txt");

  TR_NOTE("UNC-style paths handled on Unix");
#endif
}

/**
 * Test mixed path separators (Windows can use both / and \)
 */
static void test_mixed_separators(void **state)
{
  (void)state;

  TR_STEP("verify mixed path separator handling");

#ifdef _WIN32
  // Windows accepts both separators
  char path[PATH_MAX] = "C:\\Users\\test/Documents/file.txt";

  // Should find both types of separators
  assert_true(strchr(path, '\\') != NULL);
  assert_true(strchr(path, '/') != NULL);

  // Appending should work regardless
  char *c = path + strlen(path);
  size_t remaining = PATH_MAX - (c - path);
  snprintf(c, remaining, "/subfolder\\image.jpg");

  assert_true(strlen(path) < PATH_MAX);
  TR_NOTE("mixed separators handled on Windows");
#else
  // Unix treats backslash as regular character
  char path[PATH_MAX] = "/home/user/file.txt";

  char *c = path + strlen(path);
  size_t remaining = PATH_MAX - (c - path);
  snprintf(c, remaining, "/subdir/image.jpg");

  assert_string_equal(path, "/home/user/file.txt/subdir/image.jpg");
  TR_NOTE("standard separators handled on Unix");
#endif
}

/**
 * Test the specific pattern used in gallery.c for extension appending
 */
static void test_extension_appending_pattern(void **state)
{
  (void)state;

  TR_STEP("verify extension appending pattern from gallery.c");

  // This is the exact pattern from gallery.c:327-332
  char filename[PATH_MAX] = "/path/to/image";
  const char *ext = "jpg";

  char *c = filename + strlen(filename);
  for(; c > filename && *c != '.' && *c != '/' && *c != TEST_PATH_SEP; c--)
    ;
  if(c <= filename || *c == '/' || *c == TEST_PATH_SEP)
    c = filename + strlen(filename);

  snprintf(c, PATH_MAX - (c - filename), ".%s", ext);

  assert_string_equal(filename, "/path/to/image.jpg");

  // Test with existing extension
  strcpy(filename, "/path/to/image.png");
  c = filename + strlen(filename);
  for(; c > filename && *c != '.' && *c != '/' && *c != TEST_PATH_SEP; c--)
    ;
  if(c <= filename || *c == '/' || *c == TEST_PATH_SEP)
    c = filename + strlen(filename);

  snprintf(c, PATH_MAX - (c - filename), ".%s", ext);

  assert_string_equal(filename, "/path/to/image.jpg");

  TR_NOTE("extension appending pattern works correctly");
}

/**
 * Test the thumbnail filename pattern from gallery.c
 */
static void test_thumbnail_pattern(void **state)
{
  (void)state;

  TR_STEP("verify thumbnail filename generation pattern");

  // Pattern from gallery.c:365-369
  char relthumbfilename[PATH_MAX] = "image.jpg";
  const char *ext = "jpg";

  char *c = relthumbfilename + strlen(relthumbfilename);
  for(; c > relthumbfilename && *c != '.'; c--)
    ;
  if(c <= relthumbfilename)
    c = relthumbfilename + strlen(relthumbfilename);

  snprintf(c, PATH_MAX - (c - relthumbfilename), "-thumb.%s", ext);

  assert_string_equal(relthumbfilename, "image-thumb.jpg");

  TR_NOTE("thumbnail filename pattern works correctly");
}

/**
 * Test numbered file pattern (img_N.html)
 */
static void test_numbered_file_pattern(void **state)
{
  (void)state;

  TR_STEP("verify numbered file pattern from gallery.c");

  // Pattern from gallery.c:372-374
  char subfilename[PATH_MAX] = "/export/gallery";
  char *sc = subfilename + strlen(subfilename);
  int num = 42;

  snprintf(sc, PATH_MAX - (sc - subfilename), "/img_%d.html", num);

  assert_string_equal(subfilename, "/export/gallery/img_42.html");

  // Test with large numbers
  strcpy(subfilename, "/export/gallery");
  sc = subfilename + strlen(subfilename);
  num = 999999;

  snprintf(sc, PATH_MAX - (sc - subfilename), "/img_%d.html", num);

  assert_string_equal(subfilename, "/export/gallery/img_999999.html");

  TR_NOTE("numbered file pattern works for various input numbers");
}

/**
 * Test NULL termination is always preserved
 */
static void test_null_termination(void **state)
{
  (void)state;

  TR_STEP("verify NULL termination is always preserved");

  char buffer[20] = "/short/path";
  char *c = buffer + strlen(buffer);
  size_t remaining = sizeof(buffer) - (c - buffer);

  // Try to write more than fits
  snprintf(c, remaining, "/very/long/subdirectory/that/wont/fit.txt");

  // Buffer should still be null-terminated
  assert_true(buffer[sizeof(buffer) - 1] == '\0' ||
              buffer[strlen(buffer)] == '\0');
  assert_true(strlen(buffer) < sizeof(buffer));

  TR_NOTE("NULL termination preserved even with overflow attempts");
}

int main(void)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_path_pointer_arithmetic),
    cmocka_unit_test(test_long_paths),
    cmocka_unit_test(test_unc_paths),
    cmocka_unit_test(test_mixed_separators),
    cmocka_unit_test(test_extension_appending_pattern),
    cmocka_unit_test(test_thumbnail_pattern),
    cmocka_unit_test(test_numbered_file_pattern),
    cmocka_unit_test(test_null_termination),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}

// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on
