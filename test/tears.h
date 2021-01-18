#include <stdio.h>
#include "readpng.h"
#include "munit.h"
#include "../src/mapping.h"

void test2teardown(void* fixture);

void* test3setup(const MunitParameter params[], void* userdata);
void test3teardown(void* fixture);

typedef struct {
    image img;
    chunkmap map;
} test4stuff;
void* test4setup(const MunitParameter params[], void* userdata);
void test4teardown(void* fixture);


typedef struct {
    image img;
    FILE* fp;
} test5stuff;
void* test5setup(const MunitParameter params[], void* userdata);
void test5teardown(void* fixture);

typedef struct
{
    image img;
    chunkmap map;
    NSVGimage* svg;
} test7stuff;

void* test7setup(const MunitParameter params[], void* userdata);
void test7teardown(void* fixture);