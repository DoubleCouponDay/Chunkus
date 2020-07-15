
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <nanosvg.h>
#include <jpeglib.h>

const char* format1 = "png";
const char* format2 = "jpeg";

void 

int main(int arg1, char** args2)
{	
	char* possiblefileaddress = args2[1];
	
	if(possiblefileaddress == NULL) {
		printf("ERROR: path to image not given. \n");
		printf("supported formats: %s, %s \n", format1, format2);
		exit(-1);
	}
	printf("turning %s into a vector... \n", possiblefileaddress);
	printf("program completed. \n");
	return 0;
}
