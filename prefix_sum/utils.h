#ifndef OPENCL_HOMEWORKS_UTILS_H
#define OPENCL_HOMEWORKS_UTILS_H

#define MILLIS_IN_SECOND 1000

float *generateMatrix(int a, int b);

float *getTransposedMatrix(float const *matrix, int a, int b);

int matrixCompare(float const *matrix1, float const *matrix2, int a, int b);

char *readFile(char const *fileName, size_t *programSize);

#endif //OPENCL_HOMEWORKS_UTILS_H
