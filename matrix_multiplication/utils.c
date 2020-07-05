#include <stdlib.h>
#include <stdio.h>
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
    if (!matrix1 || !matrix2) {
        return 0;
    }
    int i;
    for (i = 0; i < a * b && matrix1[i] == matrix2[i]; ++i);

    return i == a * b;
}


char *readFile(char const *fileName, size_t *programSize) {
    // TODO сделать нормальное чтение
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *program = (char *)malloc(fileSize * sizeof(char) + 2);
    char ch = 0;
    *programSize = 0;
    while ((ch = fgetc(fp)) != EOF) {
        program[(*programSize)++] = ch;
    }
    program[*programSize] = 0;
    (*programSize)++;
    fclose(fp);
    return program;
}
