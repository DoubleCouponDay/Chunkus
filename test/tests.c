#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#define NANOSVG_IMPLEMENTATION

static MunitResult itCanConvertAPng() {
  //decompress the png
  //detect line boundaries
    //figure out the definition of a line boundary
    //how much difference between colours
  //define a shape in svg based on the line boundary
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  MunitTest test1 = { "test", itCanConvertAPng };
  MunitTest testarray[] = { test1, {} };
  MunitSuite suite1 = { "tests.", testarray };
  return munit_suite_main(&suite1, NULL, sizeof(testarray) / sizeof(MunitTest), argv);
}
