#include <stdio.h>
#include <assert.h>
#include "munit.h"
#include "../src/main.h"

static MunitResult test(const MunitParameter params[], void* fixture) {
  char* str = (char*) fixture;
  munit_assert_string_equal(str, "Hello, world!");
  return MUNIT_OK;
}

static MunitSuite suite = {
  
};

int main(int argc, char** argv) {
  return munit_suite_main(&suite, NULL, argc, argv);
}