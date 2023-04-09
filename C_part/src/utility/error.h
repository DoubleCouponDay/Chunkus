#pragma once

enum vectorizing_opcodes {
    SUCCESS_CODE,
    ASSUMPTION_WRONG,
    TEMPLATE_FILE_NOT_FOUND,
    SVG_SPACE_ERROR,
    READ_FILE_ERROR,
    ARRAY_DIFF_SIZE_ERROR,
    NULL_ARGUMENT_ERROR,
    OVERFLOW_ERROR,
    BAD_ARGUMENT_ERROR,
    NOT_PNG_OR_JPEG,
    LOW_BOUNDARIES_CREATED,
    RGBA_UNSUPPORTED,
    GREYSCALE_UNSUPPORTED,
    CANT_LOG
};

int isBadError();
int getLastError();
void setError(int error);
int getAndResetErrorCode();
