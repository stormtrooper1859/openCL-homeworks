#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_prefix_sum.h"
#include "openCL_prefix_sum.h"


int main() {
    srand(time(0));

    const size_t n = 1024 * 1024 * 64; // 325 ms

    clock_t tc0 = clock();

    float *vector = generateMatrix(n, 1);

    clock_t tc1 = clock();


    float stat0 = (float) (tc1 - tc0) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("generate test data time: %.0fms\n", stat0);

//    float *vectorResultCL = NULL;
    float *vectorResultCL = prefixSumOpenCL(vector, n);

    clock_t begin_calculation = clock();

    printf("comparing...\n");

    float *vectorResult = prefixSum(vector, n);

    clock_t end_calculation = clock();


//    printMatrix(vector, 1, n);
//    printMatrix(vectorResultCL, 1, n);
//    printMatrix(vectorResult, 1, n);

    if (vectorResultCL != NULL && vectorResult != NULL) {
        printf("%0.6f %0.6f\n", vectorResultCL[n-1], vectorResult[n-1]);
    }

    float stat1 = (float) (end_calculation - begin_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat1);

    printf("Result of comparing: %i\n", matrixCompare(vectorResultCL, vectorResult, n, 1));

    return 0;
}
