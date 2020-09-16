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


struct PrefixSumContext {
    cl_kernel kernelPrefix;
    cl_kernel kernelAdd;
    cl_command_queue commandQueue;
    cl_context context;
    size_t local_group_size;
    cl_event *firstEvent;
    cl_event *lastEvent;
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
    const char *programSource = readFile("./prefix_sum.cl", &programSize);
    if (!programSource) {
        goto release_context;
    }

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
        char *buildInfoLog = (char *) malloc(sizeof(char) * clBuildInfoLogSize);
        clGetProgramBuildInfo(program, deviceIds[numOfDevice], CL_PROGRAM_BUILD_LOG, clBuildInfoLogSize, buildInfoLog,
                              &clBuildInfoLogSize);
        printf("Compiler response: %s", buildInfoLog);
        free(buildInfoLog);
    }

    CHECK_ERR("clBuildProgram", errCode, release_program);
    DEBUG_PRINT(printf("Program has been built successfully"));


    const char prefixSumString[] = "prefix_sum";
    cl_kernel kernelPrefixSum = clCreateKernel(program, prefixSumString, &errCode);
    CHECK_ERR("clCreateKernel prefix_sum", errCode, release_program);

    const char addToBlocksString[] = "add_to_blocks";
    cl_kernel kernelAddToBlocks = clCreateKernel(program, addToBlocksString, &errCode);
    CHECK_ERR("clCreateKernel add_to_blocks", errCode, release_kernel_prefixSum);


    props = (struct PrefixSumContext *) malloc(sizeof(struct PrefixSumContext));
    props->kernelPrefix = kernelPrefixSum;
    props->kernelAdd = kernelAddToBlocks;
    props->commandQueue = commandQueue;
    props->context = context;
    props->local_group_size = LOCAL_GROUP_SIZE; // TODO calculate better tile_size
    props->firstEvent = malloc(sizeof(cl_event) * 10);
    props->lastEvent = props->firstEvent;

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

// +
cl_int prefixSumOpenCLInner(struct PrefixSumContext *prefixSumContext, cl_mem bufferData, size_t dataSize) {
    cl_int errCode;

    // делаем размер work_size кратным local_group_size
    size_t workSize = dataSize;
    if (workSize % prefixSumContext->local_group_size != 0) {
        workSize = workSize - workSize % prefixSumContext->local_group_size + prefixSumContext->local_group_size;
    }
    size_t workSizeDim[1] = {workSize};
    size_t localSizeDim[1] = {prefixSumContext->local_group_size};
    unsigned int blocksSumSize = workSize / prefixSumContext->local_group_size;

    cl_mem bufferBlocksSum = clCreateBuffer(prefixSumContext->context, CL_MEM_READ_WRITE, sizeof(float) * blocksSumSize,
                                            NULL, &errCode);

    errCode = clSetKernelArg(prefixSumContext->kernelPrefix, 0, sizeof(cl_mem), &bufferData);
    errCode = clSetKernelArg(prefixSumContext->kernelPrefix, 1, sizeof(cl_mem), &bufferBlocksSum);
    errCode = clSetKernelArg(prefixSumContext->kernelPrefix, 2, sizeof(int), &dataSize);
    CHECK_ERR("clSetKernelArg kernelPrefix", errCode, release_buffer_blocks);


    errCode = clEnqueueNDRangeKernel(prefixSumContext->commandQueue, prefixSumContext->kernelPrefix, 1, NULL,
                                     workSizeDim, localSizeDim, 0, 0, prefixSumContext->lastEvent++);
    CHECK_ERR("clEnqueueNDRangeKernel kernelPrefix", errCode, release_buffer_blocks);


    if (dataSize <= prefixSumContext->local_group_size) {
        goto release_buffer_blocks;
    }


    errCode = prefixSumOpenCLInner(prefixSumContext, bufferBlocksSum, blocksSumSize);
    if (errCode) {
        goto release_buffer_blocks;
    }


    errCode = clSetKernelArg(prefixSumContext->kernelAdd, 0, sizeof(cl_mem), &bufferData);
    errCode = clSetKernelArg(prefixSumContext->kernelAdd, 1, sizeof(cl_mem), &bufferBlocksSum);
    errCode = clSetKernelArg(prefixSumContext->kernelAdd, 2, sizeof(int), &dataSize);
    CHECK_ERR("clSetKernelArg kernelAdd", errCode, release_buffer_blocks);

    errCode = clEnqueueNDRangeKernel(prefixSumContext->commandQueue, prefixSumContext->kernelAdd, 1, NULL, workSizeDim,
                                     localSizeDim, 0, 0, prefixSumContext->lastEvent++);
    CHECK_ERR("clEnqueueNDRangeKernel kernelAdd", errCode, release_buffer_blocks);


    release_buffer_blocks:
    clReleaseMemObject(bufferBlocksSum);

    return errCode;
}

// +
void printExecutionTimeInfo(struct PrefixSumContext *prefixSumContext) {
    cl_int errCode;
    cl_ulong total_time = 0;

    errCode = clWaitForEvents(prefixSumContext->lastEvent - prefixSumContext->firstEvent, prefixSumContext->firstEvent);
    CHECK_ERR("clWaitForEvents", errCode, end);

    cl_event *current = prefixSumContext->firstEvent;

    while (current != prefixSumContext->lastEvent) {
        cl_event event = (*current++);

        cl_ulong begin, end;
        errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &begin, NULL);
        CHECK_ERR("clWaitForEvents CL_PROFILING_COMMAND_START", errCode, end);
        errCode = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
        CHECK_ERR("clWaitForEvents CL_PROFILING_COMMAND_END", errCode, end);

        total_time += end - begin;
    }

    printf("Time: %.1fms\n", total_time / 1e6);

    end:
    return;
};


float *prefixSumOpenCL(float const *inputData, size_t inputDataSize) {
    cl_int errCode;
    float *prefixSum = NULL;


    cl_device_id *deviceIds = getPreferredDevice();
    if (deviceIds == NULL) {
        goto end;
    }


    struct PrefixSumContext *prefixSumContext = getPrefixSumContext(deviceIds);
    if (prefixSumContext == NULL) {
        goto release_device;
    }


    cl_mem dataBuffer = clCreateBuffer(prefixSumContext->context, CL_MEM_READ_WRITE, sizeof(float) * inputDataSize,
                                       NULL, &errCode);
    CHECK_ERR("clCreateBuffer input data", errCode, release_context);

    errCode = clEnqueueWriteBuffer(prefixSumContext->commandQueue, dataBuffer, CL_TRUE, 0,
                                   sizeof(float) * inputDataSize, inputData, 0, NULL, NULL);
    CHECK_ERR("clEnqueueWriteBuffer input data", errCode, release_data_buffer);


    cl_int prefixSumBuffer = prefixSumOpenCLInner(prefixSumContext, dataBuffer, inputDataSize);
    if (prefixSumBuffer != 0) {
        goto release_data_buffer;
    }


    prefixSum = (float *) malloc(sizeof(float) * inputDataSize);

    errCode = clEnqueueReadBuffer(prefixSumContext->commandQueue, dataBuffer, 1, 0, sizeof(float) * inputDataSize,
                                  prefixSum, 0, NULL, NULL);
    CHECK_ERR("clEnqueueReadBuffer result data", errCode, release_prefix_sum);


    printExecutionTimeInfo(prefixSumContext);


    goto release_data_buffer;


    release_prefix_sum:
    free(prefixSum);
    release_data_buffer:
    clReleaseMemObject(dataBuffer);
    release_context:
    release_device:
    //todo release device
    end:
    return prefixSum;
}
