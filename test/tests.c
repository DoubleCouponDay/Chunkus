#include <stdio.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/testrecurse/colour.h"

static MunitResult test(const MunitParameter params[], void* fixture) {
  char* str = (char*) fixture;
  munit_assert_string_equal(str, "Hello, world!");
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  MunitTest test1 = { "test", test, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL };
  MunitTest testEnd = { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL };
  MunitTest testarray[] = { test1, testEnd };
  MunitSuite suite1 = { "tests", testarray, NULL, 1, MUNIT_SUITE_OPTION_NONE };
  return munit_suite_main(&suite1, NULL, argc, argv);
}