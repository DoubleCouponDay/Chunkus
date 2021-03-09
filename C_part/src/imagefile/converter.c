#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <jpeglib.h>

#include "../image.h"
#include "../utility/logger.h"
#include "../utility/error.h"

image convert_file_to_image(char* fileaddress) {
    LOG_INFO("picking image converter...");

    if (fileaddress == NULL) {
        LOG_ERR("fileaddress not given");
        setError(NULL_ARGUMENT_ERROR);
        return (image){NULL, 0, 0};
    }

    /// Open File
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p)
    {
        LOG_ERR("Could not open file '%s' for reading", fileaddress);
        setError(ASSUMPTION_WRONG);
        return (image){NULL, 0, 0};
    }

    LOG_INFO("Checking if file is PNG type");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        LOG_INFO("File \'%s\' was not recognised as a PNG file", fileaddress);
        setError(NOT_PNG_OR_JPEG);
        return (image){NULL, 0, 0};
    }

    else if()
}