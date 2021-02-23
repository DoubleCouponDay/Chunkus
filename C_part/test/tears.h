#pragma once

#include <stdio.h>
#include <nanosvg.h>

#include "../src/imagefile/pngfile.h"
#include "munit.h"
#include "../src/chunkmap.h"
#include "../src/nsvg/usage.h"

struct test2stuff {
  image img;
};

void* test2setup(const MunitParameter params[], void* userdata);
void test2teardown(void* fixture);

struct test4stuff {
    image img;
    chunkmap map;
};
void* test4setup(const MunitParameter params[], void* userdata);
void test4teardown(void* fixture);

struct test5stuff {
    image img;
    FILE* fp;
};
void* test5setup(const MunitParameter params[], void* userdata);
void test5teardown(void* fixture);

struct test6stuff
{
    test6stuff() : img(), nsvg_image(nullptr, free_nsvg) {}
    image img;
    nsvg_ptr nsvg_image;
};


void* test6setup(const MunitParameter params[], void* userdata);
void test6teardown(void* fixture);


struct test69stuff
{
    image img;
    chunkmap map;
};

void* test69setup(const MunitParameter params[], void* userdata);
void test69teardown(void* fixture);

struct test8stuff
{
    test8stuff() : img(), nsvg_image(nullptr, free_nsvg) {}
    image img;
    nsvg_ptr nsvg_image;
};

void* test8setup(const MunitParameter params[], void* userdata);
void test8teardown(void* fixture);

struct speedy_vectorize_stuff
{
    speedy_vectorize_stuff() : img(), nsvg_image(nullptr, free_nsvg) {}
    image img;
    nsvg_ptr nsvg_image;
};

void* speedy_vectorize_setup(const MunitParameter params[], void* userdata);
void speedy_vectorize_teardown(void* fixture);
