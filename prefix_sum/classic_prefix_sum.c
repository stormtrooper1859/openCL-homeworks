#include <stdlib.h>
#include "classic_prefix_sum.h"


float *prefixSum(float const *vector, size_t vectorSize) {
    float *resultVector = (float *) malloc(vectorSize * sizeof(float));

    resultVector[0] = vector[0];
    for (int i = 1; i < vectorSize; ++i) {
        resultVector[i] = resultVector[i - 1] + vector[i];
    }

    return resultVector;
}
