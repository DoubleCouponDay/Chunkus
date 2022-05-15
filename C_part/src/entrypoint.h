#pragma once

#include <nanosvg.h>

#ifndef VEC_EXPORT
#	if defined(__WIN32__) || defined(WIN32) || (_WIN32)
#		ifdef VEC_EXPORTDLL
#			define VEC_EXPORT __declspec(dllexport)
#		elif defined(VEC_NOEXPORTDLL)
#			define VEC_EXPORT 
#		else
#			define VEC_EXPORT 
#		endif
#	else
#		define VEC_EXPORT
#	endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "utility/error.h"
#include "chunkmap.h"

extern const int NUM_COLOURS;

typedef struct {
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
    int num_colours;
} vectorize_options;

int entrypoint(int argc, char* argv[]);
int set_algorithm(char* argv);
int just_crash();

extern const char* format1_p;
extern const char* format2_p;

typedef struct vectorizer_data
{
	const char* filename;
	const char* outputfilename;
	int chunk_size;
	float threshold;
} vectorizer_data;

// Needs memory management
typedef struct algorithm_progress
{
	vectorizer_data initial_data;
	chunkmap* map;
	int fill_x;
	int fill_y;

} algorithm_progress;

typedef struct gui_images {
    struct gui_images* previous;
    NSVGimage* current;
    struct gui_images* next;
} gui_images;

gui_images* VEC_EXPORT gui_vectorize(vectorizer_data input);

void VEC_EXPORT free_gui_images(gui_images* input);

#ifdef __cplusplus
};
#endif