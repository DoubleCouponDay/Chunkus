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
#ifndef _WIN32
#include <dirent.h>
#endif
#include "../src/tools.h"
#include "./readpng.h"

#define NANOSVG_IMPLEMENTATION
#define ERROR -1

MunitResult aTestCanPass(const MunitParameterEnum params[], void* data) {
  return MUNIT_OK;
}

void* test3setup(const MunitParameterEnum params[], void* userdata) {
  test2filesetup* setup = test2setup(params, NULL);
  return readfile(params, setup);
}

void test3teardown(void* fixture) {
  test2resources* resources = fixture;
  void* accessaVoidproperty = resources->setup;
  test2teardown(accessaVoidproperty);
  freefile(fixture); 
}

MunitResult weKnowHowToGetPixelDataFromPng3(const MunitParameterEnum params[], void* userdata) {
  DEBUG_PRINT("creating resources...\n");
  test2resources* resources = userdata;
  int x = 3, y = 9;
  png_color color, color2;
  DEBUG_PRINT("Row Pointers: %s\n", resources->row_pointers);
  color = *(png_color*)(resources->row_pointers[y] + x * 3);
  x += 1;
  color2 = *(png_color*)(resources->row_pointers[y] + x * 3);

  DEBUG_PRINT("Extracted Color: (%d, %d, %d)\n", color.red, color.green, color.blue);
  DEBUG_PRINT("Extracted 2nd Color: (%d, %d, %d)\n", color2.red, color2.green, color2.blue);

  // returns if didn't seg fault, that counts as success
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
  MunitTest test3 = { "weKnowHowToGetPixelDataFromPng3", weKnowHowToGetPixelDataFromPng3, test3setup, test3teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    test3, 
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };
  
  MunitSuite suite = { "tests.", testarray };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
