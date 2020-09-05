#pragma once

#include <stdio.h>
#include <png.h>
#include "entrypoint.h"

const char *format1 = "png";
const char *format2 = "jpeg";

int entrypoint(int argc, char* argv[]) {
    char* firstargument = argv[1];
	char* secondargument = argv[2];

	if(firstargument == NULL) {
		printf("ERROR: path to image not given. \n");
		printf("supported formats: %s, %s \n", format1, format2);
		getchar();
		return -1;
	}

	else if(!strcmp(firstargument, "-h") ||
		!strcmp(firstargument, "--help")) {
		printf("welcome to the vectorizer.exe help page.\n");
		printf("this program converts a bitmap image into a scalable vector graphic using line tracing techniques.\n");
		printf("supports the png and jpeg formats as input.\n");
		printf("arguments:\n");
		printf("the first argument should be the absolute path to your bitmap.\n");
		printf("the second argument can be -t or --test. it will run the test suite to check if all the characteristics of this program are working\n");
		printf("debug: %s \n", firstargument);
		getchar();
		return 0;
	}

	// unreachable code if first argument -h
	if(secondargument != NULL && 
		(!strcmp(secondargument, "-t") ||
		 !strcmp(secondargument, "--test"))) {
		printf("running tests instead...\n");
		getchar();
		return 0;
	}
	printf("turning %s into a vector... \n", firstargument);
	printf("program completed. \n");
	getchar();
	return 0;
}
