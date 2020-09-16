#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include "openCL_multiplication.h"
#include "utils.h"

#ifndef CHECK_ERR
#define CHECK_ERR(intro, result, exit_label)        \
do {                                                \
    if ((result) < 0)                               \
    {                                               \
        fprintf(stderr, "%s: %d", intro, result);   \
        exit_code = -1;                             \
        goto exit_label;                            \
    }                                               \
} while (0)
#endif

const size_t sizeX = 32;
const size_t sizeY = 32;
const size_t itemPerThread = 8;

#ifdef DEBUG
#ifndef DEBUG_PRINT
#define DEBUG_PRINT(s)  \
do {                    \
    s;                  \
} while (0)
#endif
#else
#define DEBUG_PRINT(s)
#endif

cl_platform_id *getPreferredPlatform() {
    cl_int errCode;
    cl_uint platformsNum;
    errCode = clGetPlatformIDs(0, NULL, &platformsNum);
    DEBUG_PRINT(printf("Platforms num: %u\n", platformsNum));

    cl_platform_id *platforms = (cl_platform_id *) malloc(platformsNum * sizeof(cl_platform_id));
    errCode = clGetPlatformIDs(platformsNum, platforms, &platformsNum);

    if (platformsNum <= 0) {
        printf("Platforms not founds\n");
        return NULL;
    }

    for (int i = 0; i < platformsNum; i++) {
        size_t clPlatformNameSize;
        errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &clPlatformNameSize);
        char *clPlatformName = (char *) malloc(clPlatformNameSize * sizeof(char));
        errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, clPlatformNameSize, clPlatformName,
                                    &clPlatformNameSize);
        DEBUG_PRINT(printf("Platform %d: %s\n", i, clPlatformName));
        free(clPlatformName);
    }

//    size_t clPlatformNameSize;
//    errCode = clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, 0, NULL, &clPlatformNameSize);
//    DEBUG_PRINT(printf("clPlatformNameSize: %llu\n", clPlatformNameSize));
//    char *clPlatformName = (char *) malloc(clPlatformNameSize * sizeof(char));
//    errCode = clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, clPlatformNameSize, clPlatformName,
//                                &clPlatformNameSize);
//    DEBUG_PRINT(printf("Platform %d: %d %s\n", 0, errCode, clPlatformName));

    return platforms;
}

cl_device_id *getPreferredDevice() {
    cl_platform_id *platforms = getPreferredPlatform();

    cl_int errCode;
    cl_uint deviceNums;
    errCode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceNums);
    DEBUG_PRINT(printf("DevicesNums: %d %d\n", errCode, deviceNums));
    cl_device_id *deviceIds = (cl_device_id *) malloc(deviceNums * sizeof(cl_device_id));
    errCode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, deviceNums, deviceIds, &deviceNums);
    DEBUG_PRINT(printf("Devices: %d %d\n", errCode, deviceNums));

    if (deviceNums <= 0) {
        printf("Devices not founds\n");
        return NULL;
    }

    for (int i = 0; i < deviceNums; i++) {
        size_t clDeviceNameSize = -1;
        errCode = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, 0, NULL, &clDeviceNameSize);
        char *clDeviceName = (char *) malloc(clDeviceNameSize * sizeof(char));
        errCode = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, clDeviceNameSize, clDeviceName, &clDeviceNameSize);
        DEBUG_PRINT(printf("DeviceName %d: %d %s\n", i, errCode, clDeviceName));
        cl_uint clDeviceAddressBits;
        size_t clDeviceAddressBitsRetSize;
        // errCode = clGetDeviceInfo(deviceIds[i], CL_DEVICE_ADDRESS_BITS, 0, NULL, &clDeviceNameSize);
        // char *clDeviceName = (char *)malloc(clDeviceNameSize * sizeof(char));
        errCode = clGetDeviceInfo(deviceIds[i], CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &clDeviceAddressBits,
                                  &clDeviceAddressBitsRetSize);
        DEBUG_PRINT(printf("DeviceAddressBits %d: %d %d\n", i, errCode, clDeviceAddressBits));
    }

    return deviceIds;
}

struct PrefixSumContext {
    cl_kernel kernel;
    cl_command_queue commandQueue;
    cl_context context;
    size_t local_group_size;
};


struct PrefixSumContext *getPrefixSumContext(cl_device_id *deviceIds) {
    cl_int errCode;
    struct PrefixSumContext *props = NULL;

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

    DEBUG_PRINT(printf("Program has been built successfully"));

    const char matrixMulString[] = "matrix_mul";
    cl_kernel kernel = clCreateKernel(program, matrixMulString, &errCode);
    CHECK_ERR("clCreateKernel kernel", errCode, release_program); //todo

    props = (struct PrefixSumContext *) malloc(sizeof(struct PrefixSumContext));
    props->kernel = kernel;
    props->commandQueue = commandQueue;
    props->context = context;
//    props->local_group_size = LOCAL_GROUP_SIZE; // TODO calculate better tile_size

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


cl_ulong printExecutionTimeInfo(cl_event event) {
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
    float *matrix3 = NULL;

    cl_device_id *deviceIds = getPreferredDevice();

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


    struct PrefixSumContext *prefixSumContext = getPrefixSumContext(deviceIds);




//    float *matrix21 = getTransposedMatrix(matrix2, k, m);
    float *matrix21 = matrix2;


    cl_mem buffer1 = clCreateBuffer(prefixSumContext->context, CL_MEM_READ_ONLY, sizeof(float) * n * k, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_1", errCode, release_context);
    cl_mem buffer2 = clCreateBuffer(prefixSumContext->context, CL_MEM_READ_ONLY, sizeof(float) * k * m, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_2", errCode, release_matrix1_buffer);
    cl_mem buffer3 = clCreateBuffer(prefixSumContext->context, CL_MEM_WRITE_ONLY, sizeof(float) * n * m, NULL, &errCode);
    CHECK_ERR("clEnqueueWriteBuffer matrix_result", errCode, release_matrix2_buffer);


    errCode = clEnqueueWriteBuffer(prefixSumContext->commandQueue, buffer1, 1, 0, sizeof(float) * n * k, matrix1, 0,
                                   NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix1", errCode, release_matrix3_buffer);
    errCode = clEnqueueWriteBuffer(prefixSumContext->commandQueue, buffer2, 1, 0, sizeof(float) * k * m, matrix21, 0,
                                   NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer matrix2", errCode, release_matrix3_buffer);

    errCode = clSetKernelArg(prefixSumContext->kernel, 0, sizeof(cl_mem), &buffer1);
    errCode = clSetKernelArg(prefixSumContext->kernel, 1, sizeof(cl_mem), &buffer2);
    errCode = clSetKernelArg(prefixSumContext->kernel, 2, sizeof(cl_mem), &buffer3);
    errCode = clSetKernelArg(prefixSumContext->kernel, 3, sizeof(int), &n);
    errCode = clSetKernelArg(prefixSumContext->kernel, 4, sizeof(int), &k);
    errCode = clSetKernelArg(prefixSumContext->kernel, 5, sizeof(int), &m);
    CHECK_ERR("clSetKernelArg", errCode, release_matrix3_buffer);


    cl_event event;
    size_t dimSize[2] = {(size_t) m, (size_t) n / itemPerThread};
    size_t zero[2] = {0, 0};
    size_t dimLocal[2] = {sizeX, sizeY / itemPerThread};
    errCode = clEnqueueNDRangeKernel(prefixSumContext->commandQueue, prefixSumContext->kernel, 2, NULL, dimSize,
                                     dimLocal, 0, 0, &event);
    if (errCode != 0) {
        printf("clEnqueueNDRangeKernel errCode %d\n", errCode);
        return NULL;
    }

    matrix3 = (float *) malloc(n * m * sizeof(float));
    errCode = clEnqueueReadBuffer(prefixSumContext->commandQueue, buffer3, 1, 0, sizeof(float) * n * m, matrix3, 0, 0, 0);
    CHECK_ERR("clSetKernelArg", errCode, release_result);


    cl_ulong res = printExecutionTimeInfo(event);

    printf("Time: %lldms\n", res / 1000000);

    double flps = ((double) n * k * m * 2) / ((double) res);
    printf("%.6f GFlops\n", flps);

    goto release_matrix3_buffer;

    release_result:
    free(matrix3);
    release_matrix3_buffer:
    clReleaseMemObject(buffer3);
    release_matrix2_buffer:
    clReleaseMemObject(buffer2);
    release_matrix1_buffer:
    clReleaseMemObject(buffer1);
    release_context:
    release_device:
    end:
    return matrix3;
}
