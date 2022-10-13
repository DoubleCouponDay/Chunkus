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
#include "../src/algorithm/usage.h"
#include "tears.h"
#include "../src/utility/error.h"
#include "../src/imagefile/svg.h"
#include "../src/imagefile/converter.h"

MunitResult can_test(const MunitParameter params[], void* data);

MunitResult can_read_png(const MunitParameter params[], void* userdata);
MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata);
MunitResult can_convert_png_to_bmp(const MunitParameter params[], void* userdata);
MunitResult can_vectorize_png(const MunitParameter params[], void* userdata);
MunitResult can_convert_png_to_svg(const MunitParameter params[], void* userdata);
MunitResult can_convert_png2_to_svg(const MunitParameter params[], void* userdata);
MunitResult can_convert_png3_to_svg(const MunitParameter params[], void* userdata);

MunitResult can_read_jpeg(const MunitParameter params[], void* userdata);
MunitResult can_convert_jpeg_to_bmp(const MunitParameter params[], void* userdata);
MunitResult can_vectorize_jpeg(const MunitParameter params[], void* userdata);
MunitResult can_convert_jpeg_to_svg(const MunitParameter params[], void* userdata);
