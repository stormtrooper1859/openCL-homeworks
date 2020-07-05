#include <stdlib.h>
#include "classic_multiplication.h"

float *matrixMul(float const *matrix1, float const *matrix2, int n, int m, int p) {
    float *result_matrix = (float *) malloc(n * p * sizeof(float));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            float res = 0;
            for (int k = 0; k < m; ++k) {
                res += matrix1[i * m + k] * matrix2[k * p + j];
            }
            result_matrix[i * p + j] = res;
        }
    }

    return result_matrix;
}


float *matrixMulMP(float const *matrix1, float const *matrix2T, int n, int m, int p) {
    float *result_matrix = (float *) malloc(n * p * sizeof(float));

#pragma omp parallel
    {
        int i = 0;
        // вычисления на транспонированной матрице
#pragma omp for schedule(static, 1)
        for (i = 0; i < n; i++) {
            for (int j = 0; j < p; j++) {
                float tt = 0;
                for (int k = 0; k < m; k++) {
                    tt += matrix1[i * m + k] * matrix2T[j * m + k];
                }
                result_matrix[i * p + j] = tt;
            }
        }

    }

    return result_matrix;
}
