#pragma once

#include <stdio.h>
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

	else if(firstargument == "-h" ||
		firstargument == "--help") {
		printf("welcome to the vectorizer.exe help page.");
		printf("this program converts a bitmap image into a scalable vector graphic using line tracing techniques.");
		printf("supports the png and jpeg formats as input.");
		printf("arguments:");
		printf("the first argument should be the absolute path to your bitmap.");
		printf("the second argument can be -t or --test. it will run the test suite to check if all the characteristics of this program are working");
		getchar();
		return 0;
	}

	if(secondargument != NULL && 
		(secondargument == "-t" ||
		 secondargument == "-test")) {
		printf("running tests instead...");
		getchar();
		return 0;
	}
	printf("turning %s into a vector... \n", firstargument);
	printf("program completed. \n");
	getchar();	

	//vectorize(firstargument);

	return 0;
}
