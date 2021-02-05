#include <stdlib.h>
#include <stdbool.h>

#include "munit.h"
#include "init.h"
#include "debug.h"
#include "../src/utility/error.h"

const int ONE_TEST_SIZE = 2;

MunitTest* filtertests(namedtest* tests_array, int arraylength, char* testname) {
  DEBUG("filtering tests for '%s'\n", testname);
  MunitTest* output;
  MunitTest endofarray = { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL };

  if(testname != NULL) {
    output = calloc(1, sizeof(MunitTest) * ONE_TEST_SIZE);
    output[1] = endofarray;
    
    for(int i = 0; i < arraylength; ++i) {
        namedtest currenttest = tests_array[i];

        if(currenttest.name = testname) {
            output[0] = currenttest.test;            
            break;
        }

        else if(i == arraylength - 1) {
            DEBUG("test with name not found\n");
            exit(BAD_ARGUMENT_ERROR); //the test binary is allowed to fail
        }
    }
  }

  else {
    output = calloc(1, sizeof(MunitTest) * (arraylength + 1));

    for(int i = 0; i < arraylength; ++i) {
        output[i] = tests_array[i].test;

        if(i == arraylength - 1) {
            output[arraylength] = endofarray;
        }
    }
  }

  DEBUG("checking for access violation\n");
  MunitTest throw2 = output[0];
  char* throw = throw2.name;
  return output;
}