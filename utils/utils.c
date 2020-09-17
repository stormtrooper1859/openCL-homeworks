#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "utils.h"


static unsigned long xorshf96_x = 123456789, xorshf96_y = 362436069, xorshf96_z = 521288629;

unsigned long rng(void) {
    unsigned long t;
    xorshf96_x ^= xorshf96_x << 16;
    xorshf96_x ^= xorshf96_x >> 5;
    xorshf96_x ^= xorshf96_x << 1;

    t = xorshf96_x;
    xorshf96_x = xorshf96_y;
    xorshf96_y = xorshf96_z;
    xorshf96_z = t ^ xorshf96_x ^ xorshf96_y;

    return xorshf96_z;
}


float *generateMatrix(int a, int b) {
    float *matrix = (float *) malloc(a * b * sizeof(float));

    for (int i = 0; i < a * b; i++) {
        matrix[i] = (float) (rng() % 21) - 10;
//        matrix[i] = ((float) (rand() % 219) - 109) / 10.0f;
////        matrix[i] = ((float) (rand() % 219) - 109) / 10.0f;
//        matrix[i] = ((float) (rng() % 219) - 109) / 10.0f;
//        matrix[i] += 2.0f * (matrix[i] > 0 ? 1 : -1);
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

int floatEq(float f1, float f2) {
    return fabsf(f1 - f2) < 0.01;

    float diff = f1 - f2;
    diff *= diff < 0 ? -1 : 1;
    if (diff < 1e-5) return 1;

    f1 *= f1 < 0 ? -1 : 1;
    f2 *= f2 < 0 ? -1 : 1;
    diff /= max(f1, f2);
    diff *= diff < 0 ? -1 : 1;

    if (diff > 1e-5) {
        printf("diff %0.6f , %0.6f %0.6f\n", diff, f1, f2);
    }

    return diff < 1e-5;
};

int matrixCompare(float const *matrix1, float const *matrix2, int a, int b) {
    if (!matrix1 || !matrix2) {
        return 0;
    }
    int i;
    for (i = 0; i < a * b && floatEq(matrix1[i], matrix2[i]); ++i);

    if (i != a * b) {
        printf("index %d , %0.6f %0.6f\n", i, matrix1[i], matrix2[i]);
    }

    return i == a * b;
}


void printMatrix(float *matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%5.0f ", matrix[i * m + j]);
        }
        printf("\n");
    }
    printf("-----\n");
}


char *readFile(char const *fileName, size_t *programSize) {
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error while opening file: %s\n", fileName);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *program = (char *) malloc(sizeof(char) * (fileSize + 2));
    char ch;
    *programSize = 0;
    while ((ch = fgetc(fp)) != EOF) {
        program[(*programSize)++] = ch;
    }
    program[(*programSize)++] = 0;
    fclose(fp);
    return program;
}
