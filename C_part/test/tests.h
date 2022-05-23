#pragma once

#include <nanosvg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <png.h>
#include <errno.h>

#include "init.h"
#include "../src/utility/defines.h"
#include "munit.h"

#include "../src/chunkmap.h"
#include "../src/imagefile/pngfile.h"
#include "../src/imagefile/bmp.h"
#include "../src/entrypoint.h"
#include "../src/nsvg/usage.h"
#include "tears.h"
#include "../src/utility/error.h"
#include "../src/imagefile/svg.h"
#include "../src/imagefile/converter.h"

MunitResult aTestCanPass(const MunitParameter params[], void* data);
MunitResult can_read_png(const MunitParameter params[], void* userdata);
MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata);
MunitResult opensPngAndOutputsBmp(const MunitParameter params[], void* userdata);
MunitResult can_vectorize_image(const MunitParameter params[], void* userdata);
MunitResult can_write_chunkmap_shapes_to_file(const MunitParameter params[], void* userdata);
MunitResult can_write_to_svgfile(const MunitParameter params[], void* userdata);
MunitResult just_run(const MunitParameter params[], void* userdata);
MunitResult JPEG_to_image(const MunitParameter params[], void* userdata);
MunitResult can_convert_jpeg_to_bmp(const MunitParameter params[], void* userdata);
MunitResult jpeg_dcd(const MunitParameter params[], void* userdata);

