#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "loki/mock.h"

struct testcase {
    const char *name;
    void (*func)(void);
    int success;
    char details[10000];
    struct testcase *next;
};

struct testsuite {
    const char *name;
    int num_tests;
    int num_failures;
    struct testcase *tests;
    struct testcase *current;
} testsuite;

void testsuite_write_xml_report(struct testsuite *suite)
{
    FILE *fp = stdout;
    fprintf(fp, "<testsuite name=\"%s\" tests=\"%d\" failures=\"%d\">\n",
            suite->name, suite->num_tests, suite->num_failures);
    struct testcase *test = suite->tests;
    while (test) {
        fprintf(fp, "  <testcase name=\"%s\"%s>\n", test->name, test->success ? "/" : "");
        if (!test->success) {
            fprintf(fp, "    <failure>%s</failure>\n", test->details);
            fprintf(fp, "  </testcase>\n");
        }
        test = test->next;
    }
    fprintf(fp, "</testsuite>\n");
}

void testsuite_add_test(struct testsuite *suite, void (*testfunc)(), const char *testname)
{
    struct testcase *test = (struct testcase *) malloc(sizeof(struct testcase));
    test->func = testfunc;
    test->name = testname;
    test->success = 1;
    test->next = NULL;
    test->details[0] = 0;
    suite->num_tests++;
    if (suite->tests) {
        struct testcase *last = suite->tests;
        while (last->next) {
            last = last->next;
        }
        last->next = test;
    } else {
        suite->tests = test;
    }
}

void testsuite_start(struct testsuite *suite)
{
    printf("Starting suite %s\n", suite->name);
}

void testsuite_start_test(struct testsuite *suite, struct testcase *test)
{
    suite->current = test;
    printf("Starting test %s\n", test->name);
}

void testsuite_record_fail(struct testsuite *suite, const char *msgformat, ...)
{
    suite->current->success = 0;
    size_t offset = strlen(suite->current->details);
    if (offset + strlen(msgformat) + 100 > sizeof(suite->current->details)) {
        return;
    }
    va_list args;
    va_start(args, msgformat);
    vsprintf(&suite->current->details[offset], msgformat, args);
    va_end(args);
}

void testsuite_end_test(struct testsuite *suite, struct testcase *test)
{
    if (!test->success) {
        suite->num_failures++;
        printf("%s\n", test->details);
    }
    printf("  %s\n", test->success ? "OK" : "FAIL");
}

void testsuite_end(struct testsuite *suite)
{
    printf("End suite %s\n", suite->name);
    testsuite_write_xml_report(suite);
}

#define RUN_TESTS(suitename, block) \
int main()\
{\
    testsuite.name = #suitename;\
    block\
    struct testcase *test = testsuite.tests;\
    testsuite_start(&testsuite);\
    while (test) {\
        testsuite_start_test(&testsuite, test);\
        setup_mocks();\
        test->func();\
        testsuite_end_test(&testsuite, test);\
        test = test->next;\
    }\
    testsuite_end(&testsuite);\
    return testsuite.num_failures;\
}

#define ADD_TEST(f) testsuite_add_test(&testsuite, f, #f);



void assert_eq(int expected, int actual)
{
    if (actual != expected) {
        testsuite_record_fail(&testsuite, "Integers not equal: expected <%d>, actual: <%d>\n", expected, actual);
    }
}

void assert_eq_u(unsigned int expected, unsigned int actual)
{
    if (actual != expected) {
        testsuite_record_fail(&testsuite, "Integers not equal: expected <%u>, actual: <%u>\n", expected, actual);
    }
}

void assert_not_eq(int expected, int actual)
{
    if (actual == expected) {
        testsuite_record_fail(&testsuite, "Integers equal: expected not <%d>, actual: <%d>\n", expected, actual);
    }
}

void assert_eq_string(const char *expected, const char *actual)
{
    if (0 != strcmp(expected, actual)) {
        testsuite_record_fail(&testsuite, "Strings not equal: expected <%s>, actual: <%s>\n", expected, actual);
    }
}

void assert_true(int actual)
{
    if (!actual) {
        testsuite_record_fail(&testsuite, "Value expected to be true but was: <%d>\n", actual);
    }
}

void assert_false(int actual)
{
    if (actual) {
        testsuite_record_fail(&testsuite, "Value expected to be false but was: <%d>\n", actual);
    }
}

