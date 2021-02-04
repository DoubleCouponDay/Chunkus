#pragma once

enum vectorizing_opcodes {
    SUCCESS_CODE,
    ASSUMPTION_WRONG,
    TEMPLATE_FILE_NOT_FOUND,
    SVG_SPACE_ERROR,
    READ_FILE_ERROR,
    ARRAY_DIFF_SIZE_ERROR,
    NULL_ARGUMENT_ERROR,
    HASHMAP_OOM,
    OVERFLOW_ERROR,
    BAD_ARGUMENT_ERROR,
    NOT_PNG
};

int getLastError();
void setError(int error);
int getAndResetErrorCode();
