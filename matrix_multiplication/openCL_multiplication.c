#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include "utils.h"
#include "openCL_utils.h"

#include "openCL_multiplication.h"


const size_t localGroupSize = 32;
const size_t itemsPerThread = 8;


struct MatrixMultiplicationContext {
    cl_kernel kernel;
    cl_command_queue commandQueue;
    cl_context context;
    cl_program program;
};


struct MatrixMultiplicationContext *getMatrixMultiplicationContext(cl_device_id deviceId) {
    cl_int errCode;
    struct MatrixMultiplicationContext *matrixMultiplicationContext = NULL;

    cl_uint deviceNumbers = 1;
    cl_context context = clCreateContext(NULL, deviceNumbers, &deviceId, NULL, NULL, &errCode);
    CHECK_ERR("clCreateContext", errCode, end);


    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceId, CL_QUEUE_PROFILING_ENABLE, &errCode);
    CHECK_ERR("clCreateCommandQueue", errCode, release_context);


    size_t programSize = 0;
    const char *programSource = readFile("./matrix_multiplication.cl", &programSize);
    if (!programSource) {
        goto release_context;
    }


    cl_program program = clCreateProgramWithSource(context, 1, &programSource, &programSize, &errCode);
    free(programSource);
    CHECK_ERR("clCreateProgramWithSource", errCode, release_queue);

    char buildOptions[1000];
    sprintf(buildOptions, "-D TILE_W=%Iu -D WPT=%Iu", localGroupSize, itemsPerThread);
    DEBUG_PRINT(printf("kernel build options: %s\n", buildOptions));


    errCode = clBuildProgram(program, deviceNumbers, &deviceId, buildOptions, NULL, NULL);

    // выводим лог сборки если произошла ошибка или мы в дебаг режиме
    if (errCode == CL_BUILD_PROGRAM_FAILURE
        #ifdef DEBUG
        || errCode == 0
#endif
            ) {
        size_t clBuildInfoLogSize = 0;
        clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &clBuildInfoLogSize);
        char *buildInfoLog = malloc(sizeof(char) * clBuildInfoLogSize);
        clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, clBuildInfoLogSize, buildInfoLog, &clBuildInfoLogSize);
        printf("Compiler response: %s\n", buildInfoLog);
        free(buildInfoLog);
    }

    CHECK_ERR("clBuildProgram", errCode, release_program);

    DEBUG_PRINT(printf("Program has been built successfully\n"));

    const char matrixMulString[] = "matrix_mul";
    cl_kernel kernel = clCreateKernel(program, matrixMulString, &errCode);
    CHECK_ERR("clCreateKernel kernel", errCode, release_program);

    matrixMultiplicationContext = malloc(sizeof(struct MatrixMultiplicationContext));
    matrixMultiplicationContext->kernel = kernel;
    matrixMultiplicationContext->commandQueue = commandQueue;
    matrixMultiplicationContext->context = context;
    matrixMultiplicationContext->program = program;

    goto end;

release_program:
    clReleaseProgram(program);
release_queue:
    clReleaseCommandQueue(commandQueue);
release_context:
    clReleaseContext(context);
end:
    return matrixMultiplicationContext;
}


void releaseMatrixMultiplicationContext(struct MatrixMultiplicationContext *matrixMulContext) {
    clReleaseKernel(matrixMulContext->kernel);
    clReleaseProgram(matrixMulContext->program);
    clReleaseCommandQueue(matrixMulContext->commandQueue);
    clReleaseContext(matrixMulContext->context);
    free(matrixMulContext);
};


cl_ulong getExecutionTimeInfo(cl_event event) {
    cl_int errCode;
    cl_ulong begin = 0, end = 0;

    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &begin, NULL);
    CHECK_ERR("clGetEventProfilingInfo start time", errCode, end);
    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    CHECK_ERR("clGetEventProfilingInfo end time", errCode, end);

end:
    return end - begin;
}


float *matrixMulOpenCL(float const *matrix1, float const *matrix2, size_t n, size_t k, size_t m) {
    cl_int errCode;
    float *resultMatrix = NULL;

    cl_device_id deviceIds = getPreferredDevice(CL_DEVICE_TYPE_GPU);
    if (deviceIds == NULL) {
        goto end;
    }


    struct MatrixMultiplicationContext *matrixMulContext = getMatrixMultiplicationContext(deviceIds);
    if (deviceIds == NULL) {
        goto release_device;
    }


    cl_mem bufferMatrix1 = clCreateBuffer(matrixMulContext->context, CL_MEM_READ_ONLY, sizeof(float) * n * k, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_1", errCode, release_context);
    cl_mem bufferMatrix2 = clCreateBuffer(matrixMulContext->context, CL_MEM_READ_ONLY, sizeof(float) * k * m, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_2", errCode, release_matrix1_buffer);
    cl_mem bufferResultMatrix = clCreateBuffer(matrixMulContext->context, CL_MEM_WRITE_ONLY, sizeof(float) * n * m, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_result", errCode, release_matrix2_buffer);


    errCode = clEnqueueWriteBuffer(matrixMulContext->commandQueue, bufferMatrix1, CL_TRUE, 0, sizeof(float) * n * k, matrix1, 0, NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix1", errCode, release_matrix3_buffer);
    errCode = clEnqueueWriteBuffer(matrixMulContext->commandQueue, bufferMatrix2, CL_TRUE, 0, sizeof(float) * k * m, matrix2, 0, NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix2", errCode, release_matrix3_buffer);

    errCode = clSetKernelArg(matrixMulContext->kernel, 0, sizeof(cl_mem), &bufferMatrix1);
    errCode = clSetKernelArg(matrixMulContext->kernel, 1, sizeof(cl_mem), &bufferMatrix2);
    errCode = clSetKernelArg(matrixMulContext->kernel, 2, sizeof(cl_mem), &bufferResultMatrix);
    errCode = clSetKernelArg(matrixMulContext->kernel, 3, sizeof(int), &n);
    errCode = clSetKernelArg(matrixMulContext->kernel, 4, sizeof(int), &k);
    errCode = clSetKernelArg(matrixMulContext->kernel, 5, sizeof(int), &m);
    CHECK_ERR("clSetKernelArg", errCode, release_matrix3_buffer);


    cl_event event;
    size_t workSizeDim[2] = {m, n / itemsPerThread};
    size_t localSizeDim[2] = {localGroupSize, localGroupSize / itemsPerThread};
    errCode = clEnqueueNDRangeKernel(matrixMulContext->commandQueue, matrixMulContext->kernel, 2, NULL, workSizeDim, localSizeDim, 0, 0, &event);
    CHECK_ERR("clEnqueueNDRangeKernel kernelPrefix", errCode, release_matrix3_buffer);

    clWaitForEvents(1, &event);

    resultMatrix = (float *) malloc(n * m * sizeof(float));
    errCode = clEnqueueReadBuffer(matrixMulContext->commandQueue, bufferResultMatrix, CL_TRUE, 0, sizeof(float) * n * m, resultMatrix, 0, 0, 0);
    if (errCode) {
        free(resultMatrix);
        resultMatrix = NULL;
    }
    CHECK_ERR("clSetKernelArg", errCode, release_matrix3_buffer);


    cl_ulong executionTime = getExecutionTimeInfo(event);
    printf("OpenCL matrix multiplication time: %lldms\n", executionTime / 1000000);

    double gflops = ((double) n * k * m * 2) / ((double) executionTime);
    printf("Performance %.1f GFlops\n", gflops);


release_matrix3_buffer:
    clReleaseMemObject(bufferResultMatrix);
release_matrix2_buffer:
    clReleaseMemObject(bufferMatrix2);
release_matrix1_buffer:
    clReleaseMemObject(bufferMatrix1);
release_context:
    releaseMatrixMultiplicationContext(matrixMulContext);
release_device:
    clReleaseDevice(deviceIds);
end:
    return resultMatrix;
}
