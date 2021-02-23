#pragma once

#ifdef _WIN32
#define DEBUG(fmt, ...) printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define DEBUG(fmt, args...) printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#endif
