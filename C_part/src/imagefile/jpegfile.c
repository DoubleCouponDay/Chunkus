#include "jpegfile.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "turbojpeg.h"

#include "../image.h"
#include "../utility/error.h"
#include "../utility/logger.h"

bool file_is_jpeg(char* fileaddress) {
	char current = fileaddress[0];
	int index = 0;

	while(current != NULL) {		
		index++;
		current = fileaddress[index];
	}
	char secondlast = fileaddress[index - 1];
	char thirdlast = fileaddress[index - 2];
	char fourthlast = fileaddress[index - 3];
	char fifthlast = fileaddress[index - 4];

	if((fifthlast == 'j' && fourthlast == 'p' && thirdlast == 'e' && secondlast == 'g') ||
		(fourthlast == 'j' && thirdlast == 'p' && secondlast == 'g'))
		return true;

	return false;
}
