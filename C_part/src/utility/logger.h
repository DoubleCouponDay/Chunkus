#pragma once

void clear_logfile();
void logger(const char* tag, const char* message, ...);

#ifdef _MSC_VER

#define LOG_INFO(fmt, ...) logger("INFO", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) logger("WARNING", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) logger("ERROR", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, args...) logger("INFO", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define LOG_WARN(fmt, args...) logger("WARNING", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define LOG_ERR(fmt, args...) logger("ERROR", "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#endif
