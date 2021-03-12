#include <stdlib.h>
#include <stdbool.h>

#include "munit.h"
#include "init.h"

#include "../src/utility/error.h"
#include "../src/utility/logger.h"

const int ONE_TEST_SIZE = 2;

MunitTest* filtertests(namedtest* tests_array, int arraylength, char* testname) {
  LOG_INFO("filtering tests for '%s'", testname);
  MunitTest* output;
  MunitTest endofarray = { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL };

  if(testname != NULL) {
    output = calloc(1, sizeof(MunitTest) * ONE_TEST_SIZE);
    output[1] = endofarray;
    
    for(int i = 0; i < arraylength; ++i) {
        namedtest currenttest = tests_array[i];

        if(strcmp(currenttest.name, testname) == 0) {  //fixed
            output[0] = currenttest.test;            
            break;
        }

        else if(i == arraylength - 1) {
            LOG_INFO("test with name not found");
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
  return output;
}