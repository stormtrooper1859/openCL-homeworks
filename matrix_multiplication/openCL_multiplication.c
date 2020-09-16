#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include "openCL_multiplication.h"
#include "utils.h"
#include "openCL_utils.h"

const size_t sizeX = 32;
const size_t sizeY = 32;
const size_t itemPerThread = 8;


struct MatrixMultiplicationContext {
    cl_kernel kernel;
    cl_command_queue commandQueue;
    cl_context context;
};


struct MatrixMultiplicationContext *getMatrixMultiplicationContext(cl_device_id *deviceIds) {
    cl_int errCode;
    struct MatrixMultiplicationContext *props = NULL;

    const int numOfDevice = 0;
    cl_uint deviceNumbers = 1;

    cl_context context = clCreateContext(NULL, deviceNumbers, deviceIds + numOfDevice, NULL, NULL, &errCode);
    CHECK_ERR("clCreateContext", errCode, exit);


    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceIds[numOfDevice], CL_QUEUE_PROFILING_ENABLE,
                                                         &errCode);
    CHECK_ERR("clCreateCommandQueue", errCode, release_context);


    size_t programSize = 0;
    const char *programSource = readFile("./matrix_multiplication.cl", &programSize);
    if (!programSource) {
        goto release_context;
    }


    cl_program program = clCreateProgramWithSource(context, 1, &programSource, &programSize, &errCode);
    CHECK_ERR("clCreateProgramWithSource", errCode, release_queue);

    char buildOptions[1000];
    sprintf(buildOptions, "-D TILE_W=%Iu -D TILE_H=%Iu -D WPT=%Iu", sizeX, sizeY, itemPerThread);

    DEBUG_PRINT(printf("kernel build options: %s\n", buildOptions));


    errCode = clBuildProgram(program, deviceNumbers, deviceIds + numOfDevice, buildOptions, NULL, NULL);

    if (errCode == CL_BUILD_PROGRAM_FAILURE
        #ifdef DEBUG
        || errCode == 0
#endif
            ) {
        size_t clBuildInfoLogSize = 0;
        clGetProgramBuildInfo(program, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, 0, NULL, &clBuildInfoLogSize);
        char *buildInfoLog = (char *) malloc(sizeof(char) * clBuildInfoLogSize);
        clGetProgramBuildInfo(program, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, clBuildInfoLogSize, buildInfoLog,
                              &clBuildInfoLogSize);
        printf("Compiler response: %s", buildInfoLog);
        free(buildInfoLog);
    }

    CHECK_ERR("clBuildProgram", errCode, release_program);

    DEBUG_PRINT(printf("Program has been built successfully\n"));

    const char matrixMulString[] = "matrix_mul";
    cl_kernel kernel = clCreateKernel(program, matrixMulString, &errCode);
    CHECK_ERR("clCreateKernel kernel", errCode, release_program); //todo

    props = (struct MatrixMultiplicationContext *) malloc(sizeof(struct MatrixMultiplicationContext));
    props->kernel = kernel;
    props->commandQueue = commandQueue;
    props->context = context;
//    props->local_group_size = LOCAL_GROUP_SIZE; // TODO calculate better tile_size



//    int a = CL_INVALID_KERNEL;
//    size_t kernelWorkGroupSize;
//    size_t actualKernelWorkGroupSizeBytes;
//    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
//                                       &kernelWorkGroupSize, &actualKernelWorkGroupSizeBytes);
//    size_t kernelPreferredWorkGroupSizeMultipe;
//    size_t actualKernelPreferredWorkGroupSizeMultipe;
//    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
//                                       sizeof(size_t),
//                                       &kernelPreferredWorkGroupSizeMultipe,
//                                       &actualKernelPreferredWorkGroupSizeMultipe);
//    cl_ulong kernelLocalMemSize;
//    long long actualKernelLocalMemSize;
//    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(long long),
//                                       &kernelLocalMemSize, &actualKernelLocalMemSize);
//    printf("GetKernelWorkGroupInfo errCode %d\n", errCode);
//    if (errCode != 0) {
//        return NULL;
//    }
//    printf("KernelWorkSize: %lu\n", kernelWorkGroupSize);
//    printf("KernelLocalMemSize: %llu\n", kernelLocalMemSize);
//    printf("KernelPreferredWorkGroupSizeMultipe: %lu\n", kernelPreferredWorkGroupSizeMultipe);




    goto exit;

    release_kernel_prefixSum:
    clReleaseKernel(kernel);
    release_program:
    clReleaseProgram(program);
    release_queue:
    free(programSource);
    clReleaseCommandQueue(commandQueue);
    release_context:
    clReleaseContext(context);
    exit:
    return props;
}

// +
void releaseMatrixMultiplicationContext(struct MatrixMultiplicationContext *matrixMulContext) {
    clReleaseKernel(matrixMulContext->kernel);
    clReleaseCommandQueue(matrixMulContext->commandQueue);
    clReleaseContext(matrixMulContext->context);
    free(matrixMulContext);
};


// +
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

    cl_device_id *deviceIds = getPreferredDevice(CL_DEVICE_TYPE_GPU);
    if (deviceIds == NULL) {
        goto end;
    }


    struct MatrixMultiplicationContext *matrixMulContext = getMatrixMultiplicationContext(deviceIds);
    if (deviceIds == NULL) {
        goto release_device;
    }

    // todo
//    float *matrix21 = getTransposedMatrix(matrix2, k, m);
    float *matrix21 = matrix2;


    cl_mem bufferMatrix1 = clCreateBuffer(matrixMulContext->context, CL_MEM_READ_ONLY, sizeof(float) * n * k, NULL,
                                          &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_1", errCode, release_context);
    cl_mem bufferMatrix2 = clCreateBuffer(matrixMulContext->context, CL_MEM_READ_ONLY, sizeof(float) * k * m, NULL,
                                          &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_2", errCode, release_matrix1_buffer);
    cl_mem bufferResultMatrix = clCreateBuffer(matrixMulContext->context, CL_MEM_WRITE_ONLY, sizeof(float) * n * m,
                                               NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_result", errCode, release_matrix2_buffer);


    errCode = clEnqueueWriteBuffer(matrixMulContext->commandQueue, bufferMatrix1, CL_TRUE, 0, sizeof(float) * n * k,
                                   matrix1, 0,
                                   NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix1", errCode, release_matrix3_buffer);
    errCode = clEnqueueWriteBuffer(matrixMulContext->commandQueue, bufferMatrix2, CL_TRUE, 0, sizeof(float) * k * m,
                                   matrix21, 0,
                                   NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix2", errCode, release_matrix3_buffer);

    errCode = clSetKernelArg(matrixMulContext->kernel, 0, sizeof(cl_mem), &bufferMatrix1);
    errCode = clSetKernelArg(matrixMulContext->kernel, 1, sizeof(cl_mem), &bufferMatrix2);
    errCode = clSetKernelArg(matrixMulContext->kernel, 2, sizeof(cl_mem), &bufferResultMatrix);
    errCode = clSetKernelArg(matrixMulContext->kernel, 3, sizeof(int), &n);
    errCode = clSetKernelArg(matrixMulContext->kernel, 4, sizeof(int), &k);
    errCode = clSetKernelArg(matrixMulContext->kernel, 5, sizeof(int), &m);
    CHECK_ERR("clSetKernelArg", errCode, release_matrix3_buffer);


    cl_event event;
    size_t workSizeDim[2] = {m, n / itemPerThread};
    size_t localSizeDim[2] = {sizeX, sizeY / itemPerThread};
    errCode = clEnqueueNDRangeKernel(matrixMulContext->commandQueue, matrixMulContext->kernel, 2, NULL, workSizeDim,
                                     localSizeDim, 0, 0, &event);
    CHECK_ERR("clEnqueueNDRangeKernel kernelPrefix", errCode, release_matrix3_buffer);

    clWaitForEvents(1, &event);

    resultMatrix = (float *) malloc(n * m * sizeof(float));
    errCode = clEnqueueReadBuffer(matrixMulContext->commandQueue, bufferResultMatrix, CL_TRUE, 0, sizeof(float) * n * m,
                                  resultMatrix, 0, 0, 0);
    CHECK_ERR("clSetKernelArg", errCode, release_result);


    cl_ulong res = getExecutionTimeInfo(event);
    printf("Time: %lldms\n", res / 1000000);

    double gflops = ((double) n * k * m * 2) / ((double) res);
    printf("%.6f GFlops\n", gflops);

    goto release_matrix3_buffer;

    release_result:
    free(resultMatrix);
    release_matrix3_buffer:
    clReleaseMemObject(bufferResultMatrix);
    release_matrix2_buffer:
    clReleaseMemObject(bufferMatrix2);
    release_matrix1_buffer:
    clReleaseMemObject(bufferMatrix1);
    release_context:
    releaseMatrixMultiplicationContext(matrixMulContext);
    release_device:
    end:
    return resultMatrix;
}
