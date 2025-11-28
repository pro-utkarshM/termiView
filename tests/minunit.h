/*
 * Copyright (c) 2012 David Siñuela Pastor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef MINUNIT_MINUNIT_H
#define MINUNIT_MINUNIT_H

#include <stdio.h>
#include <math.h>

/*
 * C++ adaptations for minunit
 */
#if defined(__cplusplus)
#include <cstdlib>
#include <cmath>
#define mu_abs(x) std::abs(x)
#else
#include <stdlib.h>
#include <math.h>
#define mu_abs(x) abs(x)
#endif

/*
 * MSVC doesn't have values for NAN, INFINITY, etc.
 * We'll define them to 0.0 to avoid compilation warnings/errors.
 */
#if defined(_MSC_VER)
# ifndef NAN
#  define NAN (0.0)
# endif
# ifndef INFINITY
#  define INFINITY (0.0)
# endif
#endif

/*
 * Minunit-style assertions!
 */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                               if (message) return message; } while (0)
#define mu_run_test_with_fixture(test, fixture) do { char *message = test(fixture); tests_run++; \
                               if (message) return message; } while (0)
#define mu_check(test) do { int res = (test); if (!res) \
    return "failed: " #test; } while (0)
#define mu_check_with_msg(test, message) do { int res = (test); if (!res) \
    return message; } while (0)

#define mu_fail(message) do { return message; } while (0)

#define mu_assert_int_eq(expected, result) do { if ((expected) != (result)) \
    return "assert_int_eq failed"; } while (0)
#define mu_assert_double_eq(expected, result) do { if (mu_abs((expected) - (result)) > 1e-9) \
    return "assert_double_eq failed"; } while(0)
#define mu_assert_string_eq(expected, result) do { if (strcmp((expected), (result))) \
    return "assert_string_eq failed"; } while(0)

/*
 * This is a simple ANSI color escape code enum.
 * It can be used to colorize the output of the tests.
 */
#if !defined(_WIN32) || defined(ANSICON)
const char* mu_color_reset = "\x1b[0m";
const char* mu_color_red = "\x1b[31m";
const char* mu_color_green = "\x1b[32m";
const char* mu_color_yellow = "\x1b[33m";
const char* mu_color_blue = "\x1b[34m";
const char* mu_color_magenta = "\x1b[35m";
const char* mu_color_cyan = "\x1b[36m";
#else
const char* mu_color_reset = "";
const char* mu_color_red = "";
const char* mu_color_green = "";
const char* mu_color_yellow = "";
const char* mu_color_blue = "";
const char* mu_color_magenta = "";
const char* mu_color_cyan = "";
#endif

/*
 * The beautiful test runner.
 */
#define mu_run_suite(suite_name) do { \
    printf("%s%s%s\n", mu_color_cyan, suite_name, mu_color_reset); \
    char *result = suite_name(); \
    if (result != 0) { \
        printf("%s[FAIL]%s %s\n", mu_color_red, mu_color_reset, result); \
    } \
    else { \
        printf("%s[PASS]%s (%d tests)\n", mu_color_green, mu_color_reset, tests_run); \
    } \
    return result != 0; \
} while (0)

int tests_run;

#endif /* MINUNIT_MINUNIT_H */
