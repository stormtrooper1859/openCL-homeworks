#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include "openCL_prefix_sum.h"
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

float *prefixSumOpenCL(float const *vector, size_t n) {
    cl_int errCode;

    cl_device_id *deviceIds = getPreferredDevice();

    const int numOfDevice = 0;

    cl_uint numd = 1;
    cl_context context = clCreateContext(NULL, numd, deviceIds + numOfDevice, NULL, NULL, &errCode);
    if (errCode != 0) {
        printf("Context errCode %d\n", errCode);
        return NULL;
    }

    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceIds[numOfDevice], CL_QUEUE_PROFILING_ENABLE,
                                                         &errCode);
    if (errCode != 0) {
        printf("CommandQueue errCode %d\n", errCode);
        return NULL;
    }


    size_t programSize = 0;
    const char *programSource = readFile("./prefix_sum.cl", &programSize);


//    printf("code: %s\n", program);
//    printf("program_size: %zu\n", programSize);



//    const char * constprogram = (const char *)program;
    cl_program clProg = clCreateProgramWithSource(context, 1, &programSource, &programSize, &errCode);
    if (errCode != 0) {
        printf("CreateProgramWithSource errCode %d\n", errCode);
        return NULL;
    }

    char *buildOptions = (char *) calloc(1, 1000 * sizeof(char));
    sprintf(buildOptions, "-D SIZE=%Iu", n);

    printf("buildOptions: %s\n", buildOptions);

    errCode = clBuildProgram(clProg, 1, deviceIds + numOfDevice, buildOptions, NULL, NULL);
    if (errCode != 0) {
        printf("BuildProgram errCode %d\n", errCode);
//        return NULL;
    }
    size_t clBuildInfoLogSize = -1;
    clGetProgramBuildInfo(clProg, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, 0, NULL, &clBuildInfoLogSize);
    char *buildInfoLog = (char *) malloc(clBuildInfoLogSize * sizeof(char));
    clGetProgramBuildInfo(clProg, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, clBuildInfoLogSize, buildInfoLog,
                          &clBuildInfoLogSize);
    printf("Compiler response: %s\n", buildInfoLog);


    const char add[] = "prefix_sum";
    errCode = 0;
    cl_kernel kernel = clCreateKernel(clProg, add, &errCode);
    if (errCode != 0) {
        printf("Kernel errCode %d\n", errCode);
        return NULL;
    }
    int a = CL_INVALID_KERNEL;
    size_t kernelWorkGroupSize;
    size_t actualKernelWorkGroupSizeBytes;
    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                                       &kernelWorkGroupSize, &actualKernelWorkGroupSizeBytes);
    size_t kernelPreferredWorkGroupSizeMultipe;
    size_t actualKernelPreferredWorkGroupSizeMultipe;
    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                       sizeof(size_t),
                                       &kernelPreferredWorkGroupSizeMultipe,
                                       &actualKernelPreferredWorkGroupSizeMultipe);
    cl_ulong kernelLocalMemSize;
    long long actualKernelLocalMemSize;
    errCode = clGetKernelWorkGroupInfo(kernel, deviceIds[numOfDevice], CL_KERNEL_LOCAL_MEM_SIZE, sizeof(long long),
                                       &kernelLocalMemSize, &actualKernelLocalMemSize);
    printf("GetKernelWorkGroupInfo errCode %d\n", errCode);
    if (errCode != 0) {
        return NULL;
    }
//    printf("KernelWorkSize: %lu\n", kernelWorkGroupSize);
//    printf("KernelLocalMemSize: %llu\n", kernelLocalMemSize);
//    printf("KernelPreferredWorkGroupSizeMultipe: %lu\n", kernelPreferredWorkGroupSizeMultipe);







    float *matrix3 = (float *) malloc(n * sizeof(float));


    cl_mem buffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * n, NULL, &errCode);
    if (errCode != 0) {
        printf("Buffer1 errCode %d\n", errCode);
        return NULL;
    }
    cl_mem buffer3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * n, NULL, &errCode);
    if (errCode != 0) {
        printf("Buffer3 errCode %d\n", errCode);
        return NULL;
    }


    errCode = clEnqueueWriteBuffer(commandQueue, buffer1, 1, 0, sizeof(float) * n, vector, 0, 0, 0);
    if (errCode != 0) {
        printf("Enqueue buffer1 errCode %d\n", errCode);
        return NULL;
    }


    errCode = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer1);
    errCode = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer3);
    errCode = clSetKernelArg(kernel, 2, sizeof(int), &n);


    cl_event event;
    //    size_t aaa = arrLen;
//    size_t dimSize[1] = {(size_t) n};
    size_t dimSize[1] = {n};
    size_t zero[1] = {0};
    size_t dimLocal[1] = {n};
    errCode = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, dimSize, dimLocal, 0, 0, &event);
    if (errCode != 0) {
        printf("clEnqueueNDRangeKernel errCode %d\n", errCode);
        return NULL;
    }

    errCode = clEnqueueReadBuffer(commandQueue, buffer3, 1, 0, sizeof(float) * n, matrix3, 0, 0, 0);
    if (errCode != 0) {
        printf("Enqueue read buffer errCode %d\n", errCode);
        return NULL;
    }


    //    int rr = 0;
    //    for (int i = 0; i < arrLen; i++){
    //        rr += c[i];
    //    }
//    printf("Result: %f\n", matrix3[0]);
    //    printMatrix(matrix1, n, k);
    //    printMatrix(matrix2, k, m);
    //    printMatrix(matrix3, n, m);



    cl_ulong begin;
    cl_ulong end;
    size_t tmp;
    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &begin, &tmp);
    if (errCode != 0) {
        printf("clGetEventProfilingInfo1 errCode %d\n", errCode);
        return NULL;
    }
    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, &tmp);
    if (errCode != 0) {
        printf("clGetEventProfilingInfo2 errCode %d\n", errCode);
        return NULL;
    }

    printf("Time: %lldms\n", (end - begin) / 1000000);
//    printf("Time begin: %lldms\n", begin);
//    printf("Time end: %lldms\n", end);

    double flps = ((double) n ) / ((double) end - begin);
    printf("%.6f GFlops\n", flps);

    errCode = clReleaseKernel(kernel);
    if (errCode != 0) {
        printf("Release kernel errCode %d\n", errCode);
        return NULL;
    }
    errCode = clReleaseProgram(clProg);
    if (errCode != 0) {
        printf("Release program errCode %d\n", errCode);
        return NULL;
    }
    errCode = clReleaseCommandQueue(commandQueue);
    if (errCode != 0) {
        printf("Release commandQueue errCode %d\n", errCode);
        return NULL;
    }
    errCode = clReleaseContext(context);
    if (errCode != 0) {
        printf("Release context errCode %d\n", errCode);
        return NULL;
    }


    end:
    return matrix3;
}

