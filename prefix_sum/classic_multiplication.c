#include <stdlib.h>
#include "classic_multiplication.h"

float *prefixSum(float const *vector, int n) {
    float *result_vector = (float *) malloc(n * sizeof(float));

    result_vector[0] = vector[0];
    for (int i = 1; i < n; ++i) {
        result_vector[i] = result_vector[i - 1] + vector[i];
    }

    return result_vector;
}
