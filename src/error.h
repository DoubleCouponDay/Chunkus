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
};

extern int error_code;

enum error_codes
{
    NONE = 0,
    NOT_PNG = 2,
};