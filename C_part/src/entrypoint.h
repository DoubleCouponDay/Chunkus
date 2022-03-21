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

typedef struct vectorizer_data
{
	const char* filename;
	int chunk_size;
	float threshold;
} vectorizer_data;

typedef struct algorithm_progress
{
	vectorizer_data initial_data;
	int big_chungus;
} algorithm_progress;

typedef struct test_struct
{
	int width;
	int height;
	unsigned char* data;
} test_struct;

void VEC_EXPORT epic_exported_function();

test_struct VEC_EXPORT get_test_struct();
void VEC_EXPORT free_test_struct();

#ifdef __cplusplus
};
#endif