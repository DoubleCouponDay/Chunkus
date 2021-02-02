#include "error.h"

#include "../test/tools.h"

int error_code = SUCCESS_CODE;

int getLastError() {
    return error_code;
}

void setError(int error) {
    DEBUG("setting error_code: %d\n", error);
    error_code = error;
}

int getAndResetErrorCode()
{
    int tmp = getLastError();
    setError(SUCCESS_CODE);
    return tmp;
}