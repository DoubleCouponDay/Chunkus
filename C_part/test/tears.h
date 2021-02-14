#pragma once

#include <stdio.h>
#include <nanosvg.h>

#include "../src/imagefile/pngfile.h"
#include "munit.h"
#include "../src/chunkmap.h"
#include "../src/nsvg/usage.h"

typedef struct {
  image img;
} test2stuff;

void test2setup(const MunitParameter params[], void* userdata);
void test2teardown(void* fixture);

typedef struct {
    image img;
    chunkmap* map;
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
    NSVGimage* nsvg_image;
} test6stuff;


void* test6setup(const MunitParameter params[], void* userdata);
void test6teardown(void* fixture);


typedef struct
{
    image img;
    chunkmap* map;
} test69stuff;

void* test69setup(const MunitParameter params[], void* userdata);
void test69teardown(void* fixture);

typedef struct
{
    image img;
    NSVGimage* nsvg_image;
} test8stuff;

void* test8setup(const MunitParameter params[], void* userdata);
void test8teardown(void* fixture);

typedef struct
{
    image img;
    NSVGimage* nsvg_image;
} speedy_vectorize_stuff;

void* speedy_vectorize_setup(const MunitParameter params[], void* userdata);
void speedy_vectorize_teardown(void* fixture);