#include <stdlib.h>
#include <CL/cl.h>
#include <stdio.h>
#include "openCL_multiplication.h"
#include "utils.h"

const size_t sizeX = 32;
const size_t sizeY = 32;
const size_t itemPerThread = 8;

char* CPU_[3] = {"nvidia", "amd", "intel"};
char* GPU_[3] = {"nvidia", "amd", "intel"};

struct openCLDevice {
    char *name;
};

struct openCLPlatform {
    cl_platform_id id;
    char *name;
    cl_uint deviceNums;
    char *devices;
};


cl_platform_id *getPreferredDevice(int deviceType) {
    deviceType = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

    cl_int errCode;
    cl_device_id *deviceIds = NULL;

    cl_uint platformsNum;
    errCode = clGetPlatformIDs(0, NULL, &platformsNum);
    DEBUG_PRINT(printf("Platforms number: %u\n", platformsNum));

    cl_platform_id *platforms = (cl_platform_id *) malloc(platformsNum * sizeof(cl_platform_id));
    errCode = clGetPlatformIDs(platformsNum, platforms, &platformsNum);
    CHECK_ERR("clGetPlatformIDs", errCode, end);

    if (platformsNum <= 0) {
        printf("Platforms not founds\n");
        goto end;
    }


//    struct openCLPlatform* clPlatform = (struct openCLPlatform *) malloc(platformsNum * sizeof(struct openCLPlatform));
//    for (int i = 0; i < platformsNum; ++i) {
//        clPlatform[i].id = platforms[i];
//    }
//    free(platforms);
//
//
//    for (int i = 0; i < platformsNum; ++i) {
//        size_t clPlatformNameSize;
//        errCode = clGetPlatformInfo(clPlatform[i].id, CL_PLATFORM_NAME, 0, NULL, &clPlatformNameSize);
//        clPlatform[i].name = (char *) malloc(clPlatformNameSize * sizeof(char));
//        errCode = clGetPlatformInfo(clPlatform[i].id, CL_PLATFORM_NAME, clPlatformNameSize, clPlatform[i].name, &clPlatformNameSize);
////        DEBUG_PRINT(printf("Platform %d: %s\n", i, clPlatformName));
////        free(clPlatformName);
//
////        cl_uint deviceNums;
//        errCode = clGetDeviceIDs(clPlatform[i].id, CL_DEVICE_TYPE_ALL, 0, NULL, &clPlatform[i].deviceNums);
//        DEBUG_PRINT(printf("DevicesNums: %d %d\n", errCode, deviceNums));
//        deviceIds = (cl_device_id *) malloc(clPlatform[i].deviceNums * sizeof(cl_device_id));
//        errCode = clGetDeviceIDs(clPlatform[i].id, CL_DEVICE_TYPE_ALL, clPlatform[i].deviceNums, deviceIds, &clPlatform[i].deviceNums);
//        DEBUG_PRINT(printf("Devices: %d %d\n", errCode, deviceNums));
//
//    }



//    int preferredPlatformIndex = 0;
//    char** preferredPlatform = deviceType == CL_DEVICE_TYPE_CPU ? CPU_ : GPU_;
//    for (preferredPlatformIndex = 0; preferredPlatformIndex < 3; ++preferredPlatformIndex) {
        for (int i = 0; i < platformsNum; i++) {
            size_t clPlatformNameSize;
            errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &clPlatformNameSize);
            char *clPlatformName = (char *) malloc(clPlatformNameSize * sizeof(char));
            errCode = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, clPlatformNameSize, clPlatformName,
                                        &clPlatformNameSize);
            DEBUG_PRINT(printf("Platform %d: %s\n", i, clPlatformName));
            free(clPlatformName);
        }
//    }


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
    deviceIds = (cl_device_id *) malloc(deviceNums * sizeof(cl_device_id));
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

    end:
    return deviceIds;
//    return clPlatform;
}


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
