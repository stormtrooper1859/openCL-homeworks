#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_multiplication.h"
#include "openCL_prefix_sum.h"

void printMatrix(float *matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%5.1f ", matrix[i * m + j]);
        }
        printf("\n");
    }
    printf("-----\n");
}


int main() {
    srand(time(0));

//    const size_t n = 2049;
//    const size_t n = 1024;
//    const size_t n = 1024 * 1024 * 10; // 325 ms
    const size_t n = 1024 * 1024 * 64; // 325 ms
//    const size_t n = 66; // 325 ms
//    const size_t n = 1024 * 1024 * 128; // 707 ms

    clock_t tc0 = clock();

    float *vector = generateMatrix(n, 1);

    clock_t tc1 = clock();


    float stat0 = (float) (tc1 - tc0) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat0);

//    float *vectorResultCL = NULL;
    float *vectorResultCL = prefixSumOpenCL(vector, n);

    clock_t begin_calculation = clock();

    printf("comparing...\n");

    float *vectorResult = prefixSum(vector, n);

    clock_t end_calculation = clock();


//    printMatrix(vector, 1, n);
//    printMatrix(vectorResultCL, 1, n);
//    printMatrix(vectorResult, 1, n);


    printf("%0.6f %0.6f\n", vectorResultCL[n-1], vectorResult[n-1]);

    float stat1 = (float) (end_calculation - begin_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat1);

    printf("Result of comparing: %i\n", matrixCompare(vectorResultCL, vectorResult, n, 1));

    return 0;
}
