#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_multiplication.h"

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

    const int n = 1000;
    const int m = 1100;
    const int p = 1200;

    float *matrix1 = generateMatrix(n, m);
    float *matrix2 = generateMatrix(m, p);

    clock_t begin_calculation = clock();

    float *matrix3 = matrixMulMP(matrix1, matrix2, n, m, p);

    clock_t end_calculation = clock();

    float *matrix3CHECK = matrixMul(matrix1, matrix2, n, m, p);

    clock_t end_calculation2 = clock();

    float stat1 = (float) (end_calculation - begin_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    float stat2 = (float) (end_calculation2 - end_calculation) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("%.0fms\n", stat1);
    printf("%.0fms\n", stat2);

    printf("Result of comparing: %i\n", matrixCompare(matrix3, matrix3CHECK, n, p));

    return 0;
}
