#include "thresholds.h"

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "../utility/logger.h"
#include "../utility/error.h"

const int MAX_THRESHOLD = 441;

//Credit to Makka_Pakka#4053
float pickon_exponential_curve(int x) {
    float plusone = (float)(MAX_THRESHOLD + 1);
    float exponent = (logf(plusone / (plusone * logf(2)))) * (float)x;
    return (float)-1 + powf(2, exponent);
}

bool outofbounds(int number) {
    if(number < 0 || number > MAX_THRESHOLD) {
        return true;
    }
    return false;
}

float* get_thresholds(int thresholds) {
    if(outofbounds(thresholds)) {
        LOG_ERR("thresholds was outside bounds! %d", thresholds);
        setError(BAD_ARGUMENT_ERROR);
        return NULL;
    }

    float* output = calloc(1, sizeof(float) * thresholds);
    
    for(int i = 0; i < thresholds; ++i) {
        int x_index = MAX_THRESHOLD / thresholds * i; //always include the most detailed threshold: 0
        float threshold = pickon_exponential_curve(x_index);

        if(outofbounds((int)threshold)) {
            LOG_ERR("exponential curve equation is wrong! %d", threshold);
            setError(ASSUMPTION_WRONG);
            return NULL;
        }
        output[i] = threshold;
    }
    return output;
}

void free_thresholds_array(float* input) {
    free(input);
}