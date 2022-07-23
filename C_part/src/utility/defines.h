#pragma once

#ifndef _WIN32
#include <dirent.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif

#ifndef NOT_FILLED
#define NOT_FILLED -1
#endif

float getpi();
