#include "error.h"

int error_code = SUCCESS_CODE;

int isBadError() {
    return error_code != SUCCESS_CODE;
}

int getLastError() {
    return error_code;
}

void setError(int error) {
    error_code = error;
}

int getAndResetErrorCode()
{
    int tmp = getLastError();
    setError(SUCCESS_CODE);
    return tmp;
}
