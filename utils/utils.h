#ifndef OPENCL_HOMEWORKS_UTILS_H
#define OPENCL_HOMEWORKS_UTILS_H

#define MILLIS_IN_SECOND 1000

#ifndef CHECK_ERR
#define CHECK_ERR(intro, result, exit_label)        \
do {                                                \
    if ((result) < 0)                               \
    {                                               \
        fprintf(stderr, "%s error code: %d\n", intro, result);   \
        goto exit_label;                            \
    }                                               \
} while (0)
#endif

#ifndef DEBUG_PRINT
#ifdef DEBUG
#define DEBUG_PRINT(s)  \
do {                    \
    s;                  \
} while (0)
#else
#define DEBUG_PRINT(s)
#endif
#endif

float *generateMatrix(int a, int b);

float *getTransposedMatrix(float const *matrix, int a, int b);

int matrixCompare(float const *matrix1, float const *matrix2, int a, int b);

void printMatrix(float *matrix, int n, int m);

char *readFile(char const *fileName, size_t *programSize);

#endif //OPENCL_HOMEWORKS_UTILS_H
