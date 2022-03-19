#pragma once

#ifndef VEC_EXPORT
#	if defined(__WIN32__) || defined(WIN32) || (_WIN32)
#		ifdef VEC_EXPORTDLL
#			define VEC_EXPORT __declspec(dllexport)
#		else
#			define VEC_EXPORT __declspec(dllimport)
#		endif
#	else
#		define VEC_EXPORT
#	endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const int NUM_COLOURS;

int entrypoint(int argc, char* argv[]);
int set_algorithm(char* argv);
int just_crash();

extern const char* format1_p;
extern const char* format2_p;

void VEC_EXPORT epic_exported_function();

#ifdef __cplusplus
};
#endif