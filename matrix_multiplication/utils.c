#include <stdlib.h>
#include "utils.h"

float *generateMatrix(int a, int b) {
    float *matrix = (float *) malloc(a * b * sizeof(float));

    for (int i = 0; i < a * b; i++) {
        matrix[i] = (float) (rand() % 21) - 10;
    }

    return matrix;
}

float *getTransposedMatrix(float const *matrix, int a, int b) {
    float *result_matrix = (float *) malloc(a * b * sizeof(float));

    for (int i = 0; i < a; ++i) {
        for (int j = 0; j < b; ++j) {
            result_matrix[j * a + i] = matrix[i * b + j];
        }
    }

    return result_matrix;
}

int matrixCompare(float const *matrix1, float const *matrix2, int a, int b) {
    int i;

    for (i = 0; i < a * b && matrix1[i] == matrix2[i]; ++i);

    return i == a * b;
}
