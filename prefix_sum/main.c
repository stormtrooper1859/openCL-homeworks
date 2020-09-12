#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_multiplication.h"
#include "openCL_prefix_sum.h"

void printMatrix(float *matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%5.0f ", matrix[i * m + j]);
        }
        printf("\n");
    }
    printf("-----\n");
}


int main() {
    srand(time(0));

    const size_t n = 1024;
//    const size_t n = 1024 * 1024 * 64; // 325 ms
//    const size_t n = 1024 * 1024 * 128; // 707 ms

    float *vector = generateMatrix(n, 1);

//    printMatrix(vector, 1, n);

    float *vectorResultCL = NULL;
//    float *vectorResultCL = prefixSumOpenCL(vector, n);

//    printMatrix(vectorResultCL, 1, n);

    clock_t begin_calculation = clock();

    printf("comparing...\n");

    float *vectorResult = prefixSum(vector, n);

//    printMatrix(vectorResult, 1, n);

    clock_t end_calculation = clock();

    float stat1 = (float) (end_calculation - begin_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat1);

    printf("Result of comparing: %i\n", matrixCompare(vectorResultCL, vectorResult, n, 1));

    return 0;
}
