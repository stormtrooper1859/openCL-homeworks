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
        fprintf(stderr, "%s error code: %d\n", intro, result);   \
        goto exit_label;                            \
    }                                               \
} while (0)
#endif

const size_t LOCAL_GROUP_SIZE = 1024;

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

cl_ulong total_time = 0;

//struct DeviceInfo {
//
//};

cl_platform_id *getPreferredDevice() {
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


void add_time(cl_event event) {
    cl_int errCode;

    errCode = clWaitForEvents(1, &event);
    if (errCode != 0) {
        printf("clWaitForEvents errCode %d\n", errCode);
        return;
    }


    cl_ulong begin;
    cl_ulong end;
    size_t tmp;
    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &begin, &tmp);
    if (errCode != 0) {
        printf("clGetEventProfilingInfo1 errCode %d\n", errCode);
        return;
    }
    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, &tmp);
    if (errCode != 0) {
        printf("clGetEventProfilingInfo2 errCode %d\n", errCode);
        return;
    }

    total_time += end - begin;

//    printf("Time: %lldms\n", (end - begin) / 1000000);

}


struct prefixSumOpenCLInnerProps {
    cl_kernel kernelPrefix;
    cl_kernel kernelAdd;
    cl_command_queue commandQueue;
    cl_context context;
    size_t tile_size;
};


cl_mem prefixSumOpenCLInner(struct prefixSumOpenCLInnerProps *props, cl_mem buffer_data, size_t n) {
    cl_int errCode;

    size_t work_size = n;
    if (work_size % props->tile_size != 0) {
        work_size = work_size - work_size % props->tile_size + props->tile_size;
    }

    cl_mem buffer_blocks_sum = clCreateBuffer(props->context, CL_MEM_READ_WRITE,
                                              sizeof(float) * (work_size / props->tile_size), NULL, &errCode);

    errCode = clSetKernelArg(props->kernelPrefix, 0, sizeof(cl_mem), &buffer_data);
    errCode = clSetKernelArg(props->kernelPrefix, 1, sizeof(cl_mem), &buffer_blocks_sum);
    errCode = clSetKernelArg(props->kernelPrefix, 2, sizeof(int), &n);

    cl_event event;


    size_t dimSize[1] = {work_size};
    size_t zero[1] = {0};
    size_t dimLocal[1] = {props->tile_size};
    errCode = clEnqueueNDRangeKernel(props->commandQueue, props->kernelPrefix, 1, NULL, dimSize, dimLocal, 0, 0,
                                     &event);
    if (errCode != 0) {
        printf("clEnqueueNDRangeKernel1 errCode %d\n", errCode);
        return NULL;
    }

    add_time(event);

    if (n <= props->tile_size) {
        return buffer_data;
    }

    // calc
    unsigned int nn = (n / props->tile_size) + (n % props->tile_size ? 1 : 0);
    cl_mem calculated = prefixSumOpenCLInner(props, buffer_blocks_sum, nn);


    errCode = clSetKernelArg(props->kernelAdd, 0, sizeof(cl_mem), &buffer_data);
    errCode = clSetKernelArg(props->kernelAdd, 1, sizeof(cl_mem), &calculated);
    errCode = clSetKernelArg(props->kernelAdd, 2, sizeof(int), &n);

    cl_event event2;
    errCode = clEnqueueNDRangeKernel(props->commandQueue, props->kernelAdd, 1, NULL, dimSize, dimLocal, 0, 0, &event2);
    if (errCode != 0) {
        printf("clEnqueueNDRangeKernel2 errCode %d\n", errCode);
        return NULL;
    }

    add_time(event2);

    clReleaseMemObject(buffer_blocks_sum);

    return buffer_data;
}


struct prefixSumOpenCLInnerProps *getContext(cl_device_id *deviceIds) {
    cl_int errCode;
    struct prefixSumOpenCLInnerProps *props = NULL;

    const int numOfDevice = 0;
    cl_uint deviceNumbers = 1;

    cl_context context = clCreateContext(NULL, deviceNumbers, deviceIds + numOfDevice, NULL, NULL, &errCode);
    CHECK_ERR("clCreateContext", errCode, exit);

    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceIds[numOfDevice], CL_QUEUE_PROFILING_ENABLE,
                                                         &errCode);
    CHECK_ERR("clCreateCommandQueue", errCode, release_context);


    size_t programSize = 0;
    const char *programSource = readFile("./prefix_sum.cl", &programSize);

    cl_program program = clCreateProgramWithSource(context, 1, &programSource, &programSize, &errCode);
    CHECK_ERR("clCreateProgramWithSource", errCode, release_queue);

    char buildOptions[1000];
    sprintf(buildOptions, "-D LOCAL_GROUP_SIZE=%Iu -Werror", LOCAL_GROUP_SIZE);


    errCode = clBuildProgram(program, deviceNumbers, deviceIds + numOfDevice, buildOptions, NULL, NULL);

    if (errCode == CL_BUILD_PROGRAM_FAILURE
        #ifdef DEBUG
        || errCode == 0
#endif
            ) {
        size_t clBuildInfoLogSize = 0;
        clGetProgramBuildInfo(program, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, 0, NULL, &clBuildInfoLogSize);
        char *buildInfoLog = (char *) malloc(clBuildInfoLogSize * sizeof(char));
        clGetProgramBuildInfo(program, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, clBuildInfoLogSize, buildInfoLog,
                              &clBuildInfoLogSize);
        printf("Compiler response: %s", buildInfoLog);
        free(buildInfoLog);
    }

    CHECK_ERR("clBuildProgram", errCode, release_program);


    const char prefixSumString[] = "prefix_sum";
    cl_kernel kernelPrefixSum = clCreateKernel(program, prefixSumString, &errCode);
    CHECK_ERR("clCreateKernel prefix_sum", errCode, release_program);

    const char addToBlocksString[] = "add_to_blocks";
    cl_kernel kernelAddToBlocks = clCreateKernel(program, addToBlocksString, &errCode);
    CHECK_ERR("clCreateKernel add_to_blocks", errCode, release_kernel_prefixSum);


    props = (struct prefixSumOpenCLInnerProps *) malloc(
            sizeof(struct prefixSumOpenCLInnerProps));
    props->kernelPrefix = kernelPrefixSum;
    props->kernelAdd = kernelAddToBlocks;
    props->commandQueue = commandQueue;
    props->context = context;
    props->tile_size = LOCAL_GROUP_SIZE;

    goto exit;

    release_kernel_prefixSum:
    clReleaseKernel(kernelPrefixSum);
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


float *prefixSumOpenCL(float const *vector, size_t n) {
    cl_int errCode;

    cl_device_id *deviceIds = getPreferredDevice();

    struct prefixSumOpenCLInnerProps *props = getContext(deviceIds);

    if (props == NULL) return NULL;

    cl_mem buffer_data = clCreateBuffer(props->context, CL_MEM_READ_WRITE, sizeof(float) * n, NULL, &errCode);

    errCode = clEnqueueWriteBuffer(props->commandQueue, buffer_data, 1, 0, sizeof(float) * n, vector, 0, 0, 0);
    if (errCode != 0) {
        printf("Enqueue buffer1 errCode %d\n", errCode);
        return NULL;
    }


    cl_mem prefixSumBuffer = prefixSumOpenCLInner(props, buffer_data, n);

    if (prefixSumBuffer == NULL) {
        return NULL;
    }


    float *prefixSum = (float *) malloc(n * sizeof(float));

    errCode = clEnqueueReadBuffer(props->commandQueue, prefixSumBuffer, 1, 0, sizeof(float) * n, prefixSum, 0, 0, 0);
    if (errCode != 0) {
        printf("Enqueue read buffer errCode %d\n", errCode);
        return NULL;
    }








//    cl_ulong begin;
//    cl_ulong end;
//    size_t tmp;
//    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &begin, &tmp);
//    if (errCode != 0) {
//        printf("clGetEventProfilingInfo1 errCode %d\n", errCode);
//        return NULL;
//    }
//    errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, &tmp);
//    if (errCode != 0) {
//        printf("clGetEventProfilingInfo2 errCode %d\n", errCode);
//        return NULL;
//    }
//
//    printf("Time: %lldms\n", (end - begin) / 1000000);
//
//    double flps = ((double) n ) / ((double) end - begin);
//    printf("%.6f GFlops\n", flps);

//    errCode = clReleaseKernel(kernel);


    printf("Time: %lldms\n", total_time / 1000000);

//    errCode = clReleaseProgram(clProg);
//    errCode = clReleaseCommandQueue(commandQueue);
//    errCode = clReleaseContext(context);


    end:
    return prefixSum;
}

