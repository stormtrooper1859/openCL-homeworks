#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "classic_multiplication.h"
#include "openCL_multiplication.h"


int main() {
    srand(time(0));

    const size_t n = 2048;
    const size_t k = 512;
    const size_t m = 1024;

    float *matrix1 = generateMatrix(n, k);
    float *matrix2 = generateMatrix(k, m);

    float *matrix3CL = matrixMulOpenCL(matrix1, matrix2, n, k, m);

    printf("compute reference matrix...\n");
    clock_t timeBeforeCalc = clock();
    float *matrix3MP = matrixMulMP(matrix1, matrix2, n, k, m);
    clock_t timeAfterCalc = clock();

    float stat1 = (float) (timeAfterCalc - timeBeforeCalc) / CLOCKS_PER_SEC * MILLIS_IN_SECOND;
    printf("CPU matrix multiplication time: %.0fms\n", stat1);

    printf("comparing...\n");
    printf("Result of comparing: %i\n", matrixCompare(matrix3CL, matrix3MP, n, m));

    return 0;
}
