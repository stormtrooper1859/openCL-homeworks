#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_multiplication.h"
#include "openCL_multiplication.h"

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

//    const size_t n = 16 * 62;
//    const size_t k = 16 * 68;
//    const size_t m = 16 * 74;
//    const size_t n = 2048;
//    const size_t k = 1024;
//    const size_t m = 2048;
    const size_t n = 2048;
    const size_t k = 512;
    const size_t m = 1024;

    float *matrix1 = generateMatrix(n, k);
    float *matrix2 = generateMatrix(k, m);

    float *matrix3CL = matrixMulOpenCL(matrix1, matrix2, n, k, m);

    clock_t begin_calculation = clock();

    printf("comparing...\n");
    float *matrix3MP = matrixMulMP(matrix1, matrix2, n, k, m);

    clock_t end_calculation = clock();

//    float *matrix3CHECK = matrixMul(matrix1, matrix2, n, m, p);

    clock_t end_calculation2 = clock();

    float stat1 = (float) (end_calculation - begin_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    float stat2 = (float) (end_calculation2 - end_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat1);
    printf("%.0fms\n", stat2);

    printf("Result of comparing: %i\n", matrixCompare(matrix3CL, matrix3MP, n, m));

    return 0;
}
