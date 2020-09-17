#include <stdlib.h>
#include "utils.h"

#include "classic_multiplication.h"


float *matrixMul(float const *matrix1, float const *matrix2, int n, int k, int m) {
    float *resultMatrix = (float *) malloc(n * m * sizeof(float));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            float res = 0;
            for (int l = 0; l < k; ++l) {
                res += matrix1[i * k + l] * matrix2[l * m + j];
            }
            resultMatrix[i * m + j] = res;
        }
    }

    return resultMatrix;
}


float *matrixMulMP(float const *matrix1, float const *matrix2, int n, int k, int m) {
    float *resultMatrix = (float *) malloc(n * m * sizeof(float));

    float *matrix2T = getTransposedMatrix(matrix2, k, m);

#pragma omp parallel
    {
        int i = 0;
        // вычисления на транспонированной матрице
#pragma omp for schedule(static, 1)
        for (i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                float tt = 0;
                for (int l = 0; l < k; l++) {
                    tt += matrix1[i * k + l] * matrix2T[j * k + l];
                }
                resultMatrix[i * m + j] = tt;
            }
        }
    }

    free(matrix2T);

    return resultMatrix;
}
