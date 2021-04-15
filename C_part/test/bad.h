#pragma once

#if defined(_DEBUG) || defined(DEBUG)
#ifdef _WIN32
#define DEBUG_OUT(fmt, ...) printf("%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define DEBUG_OUT(fmt, args...) printf("%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#endif
#else
#define DEBUG_OUT(fmt, ...)
#endif
