#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <nanosvg.h>
#include <errno.h>
#include <dirent.h>
#include "../src/tools.h"
#include "./readpng.h"

#define NANOSVG_IMPLEMENTATION
#define ERROR -1

static MunitResult aTestCanPass(const MunitParameterEnum params[], void *data) {
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  DEBUG_PRINT("test runner initializing... \n");

  char* filename;
  if (argc > 1)
    filename = argv[1];
  else
    filename = "./basic.png";
  char* filepp_params[] = { filename, NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "filepp", filepp_params
    }, 
    { NULL, NULL} 
  };
  
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", itCanDecompressAPng2, test2setup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };
  
  MunitSuite suite = { "tests.", testarray };

  return munit_suite_main(&suite, NULL, 0, argv);
  itCanDecompressAPng2(test_params, NULL);
}
