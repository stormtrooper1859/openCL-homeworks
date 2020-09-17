#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_prefix_sum.h"
#include "openCL_prefix_sum.h"


int main() {
    setRngSeed(time(0));

    const size_t n = 1024 * 1024 * 64;

    clock_t timeBeforeGen = clock();
    float *vector = generateMatrix(n, 1);
    clock_t timeAfterGen = clock();

    float stat0 = (float) (timeAfterGen - timeBeforeGen) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("Generate test data time: %.0fms\n", stat0);

    float *vectorResultCL = prefixSumOpenCL(vector, n);

    printf("compute reference prefix sum...\n");
    clock_t timeBeforeCalc = clock();
    float *vectorResult = prefixSum(vector, n);
    clock_t timeAfterCalc = clock();

    float stat1 = (float) (timeAfterCalc - timeBeforeCalc) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("CPU prefix sum computation time: %.0fms\n", stat1);

    printf("comparing...\n");
    printf("Result of comparing: %i\n", matrixCompare(vectorResultCL, vectorResult, n, 1));

    return 0;
}
