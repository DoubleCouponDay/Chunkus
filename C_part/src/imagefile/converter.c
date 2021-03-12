#include <stdlib.h>
#include <stdio.h>

#include "../image.h"
#include "../utility/logger.h"
#include "../utility/error.h"
#include "pngfile.h"
#include "jpegfile.h"

image convert_file_to_image(char* fileaddress) {
    LOG_INFO("picking image converter...");

    if (fileaddress == NULL) {
        LOG_ERR("fileaddress not given");
        setError(NULL_ARGUMENT_ERROR);
        return (image){NULL, 0, 0};
    }

    LOG_INFO("Checking if file is PNG type");
    bool ispng = file_is_png(fileaddress);

    if(isBadError()) {
        LOG_ERR("file_is_png failed");
        return (image){NULL, 0, 0};
    }

    if(ispng) {
        return convert_png_to_image(fileaddress);
    }

    LOG_INFO("checking if file is JPEG type");
    bool isjpg = file_is_jpeg(fileaddress);

    if(isBadError()) {
        LOG_ERR("file_is_jpeg failed");
        return (image){NULL, 0, 0};
    }

    if(isjpg) {
        return convert_jpeg_to_image(fileaddress);
    }

    if(isBadError()) {
        LOG_ERR("convert_jpeg_to_image failed");
        return (image){NULL, 0, 0};
    }

    LOG_INFO("File \'%s\' was not recognised as a PNG file", fileaddress);
    setError(NOT_PNG_OR_JPEG);
    return (image){NULL, 0, 0};
}