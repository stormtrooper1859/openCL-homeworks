#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "utils.h"

// fast rng https://stackoverflow.com/a/1640399
static unsigned long xorshf96_x = 123456789, xorshf96_y = 362436069, xorshf96_z = 521288629;

void setRngSeed(unsigned long seed) {
    xorshf96_x = seed;
}

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
        matrix[i] = ((float) (rng() % 219) - 109) / 10.0f; // генерируем число от -10.9 до 10.9 с шагом 0.1
        matrix[i] += (matrix[i] > 0) ? 2.0f : -2.0f; // убираю из массива числа от -2 до 2, тк float не слишком точный
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

// грубый эпсилон, тк сложении float в произвольном порядке результаты могут расходиться (особенно при подсчете префиксной суммы на больших массивах)
#define EPSILON 0.1

int floatEq(float f1, float f2) {
    return fabsf(f1 - f2) < EPSILON || fabsf((f1 - f2) / max(f1, f2)) < EPSILON;
};

int matrixCompare(float const *matrix1, float const *matrix2, int a, int b) {
    if (!matrix1 || !matrix2) {
        return 0;
    }
    int i;
    for (i = 0; i < a * b && floatEq(matrix1[i], matrix2[i]); ++i);

    if (i != a * b) {
        printf("index %d , values: %0.6f %0.6f\n", i, matrix1[i], matrix2[i]);
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
