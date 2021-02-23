#pragma once

#include "munit.h"

typedef struct {
    char* name;
    MunitTest test;
} namedtest;

MunitTest* filtertests(namedtest* tests_array, int arraylength, char* testname);
