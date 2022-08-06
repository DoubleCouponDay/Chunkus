#pragma once

#include <stdio.h>
#include <nanosvg.h>

#include "../src/imagefile/pngfile.h"
#include "munit.h"
#include "../src/chunkmap.h"
#include "../src/nsvg/usage.h"

typedef struct {
  image img;
} tear1;

void* tear1setup(const MunitParameter params[], void* userdata);
void tear1teardown(void* fixture);

typedef struct {
    image img;
    chunkmap* map;
} tear2;
void* tear2setup(const MunitParameter params[], void* userdata);
void tear2teardown(void* fixture);

typedef struct {
    image img;
    FILE* fp;
} tear3;
void* tear3setup(const MunitParameter params[], void* userdata);
void tear3teardown(void* fixture);

typedef struct
{
    image img;
    NSVGimage* nsvg_image;
} tear4;
void* tear4setup(const MunitParameter params[], void* userdata);
void tear4teardown(void* fixture);
