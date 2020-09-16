#include <CL/cl.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "utils.h"

#include "openCL_utils.h"

char *CPU_[3] = {"intel", "amd", "nvidia"};
char *GPU_[3] = {"nvidia", "amd", "intel"};

cl_device_id *getPreferredDevice(cl_device_type deviceType) {
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


    struct openCLPlatform *clPlatform = (struct openCLPlatform *) malloc(platformsNum * sizeof(struct openCLPlatform));
    for (int i = 0; i < platformsNum; ++i) {
        clPlatform[i].id = platforms[i];
    }
    // todo
//    free(platforms);


    // get platforms info
    for (int i = 0; i < platformsNum; ++i) {
        struct openCLPlatform *currentPlatform = &clPlatform[i];

        // get platform name
        size_t clPlatformNameSize;
        errCode = clGetPlatformInfo(currentPlatform->id, CL_PLATFORM_NAME, 0, NULL, &clPlatformNameSize);
        currentPlatform->name = (char *) malloc(clPlatformNameSize * sizeof(char));
        errCode = clGetPlatformInfo(currentPlatform->id, CL_PLATFORM_NAME, clPlatformNameSize, currentPlatform->name,
                                    &clPlatformNameSize);

        // get platform devices
        errCode = clGetDeviceIDs(currentPlatform->id, CL_DEVICE_TYPE_ALL, 0, NULL, &currentPlatform->deviceNums);
        cl_device_id *clDevices = (cl_device_id *) malloc(currentPlatform->deviceNums * sizeof(cl_device_id));
        errCode = clGetDeviceIDs(currentPlatform->id, CL_DEVICE_TYPE_ALL, currentPlatform->deviceNums, clDevices,
                                 &currentPlatform->deviceNums);
        currentPlatform->devices = (struct openCLDevice *) malloc(
                currentPlatform->deviceNums * sizeof(struct openCLDevice));

        for (int j = 0; j < currentPlatform->deviceNums; ++j) {
            currentPlatform->devices[j].id = clDevices[j];
        }

        free(clDevices);


        DEBUG_PRINT(printf("Found platform %s with %d devices:\n", currentPlatform->name, currentPlatform->deviceNums));


        // переводим к lower case для упрощения сравнений
        for (int k = 0; currentPlatform->name[k]; k++) {
            currentPlatform->name[k] = tolower(currentPlatform->name[k]);
        }

        // get devices info
        for (int j = 0; j < currentPlatform->deviceNums; ++j) {
            struct openCLDevice *currentDevice = &currentPlatform->devices[j];

            size_t clDeviceNameSize;
            errCode = clGetDeviceInfo(currentDevice->id, CL_DEVICE_NAME, 0, NULL, &clDeviceNameSize);
            currentDevice->name = (char *) malloc(clDeviceNameSize * sizeof(char));
            errCode = clGetDeviceInfo(currentDevice->id, CL_DEVICE_NAME, clDeviceNameSize, currentDevice->name,
                                      &clDeviceNameSize);
            DEBUG_PRINT(printf("\t %d. %s\n", j + 1, currentDevice->name));

            errCode = clGetDeviceInfo(currentDevice->id, CL_DEVICE_TYPE, sizeof(cl_device_type), &currentDevice->type,
                                      NULL);
        }
    }


    struct openCLPlatform *selectedPlatform = NULL;

    char **preferredPlatform = (deviceType & CL_DEVICE_TYPE_GPU) != 0 ? GPU_ : CPU_;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < platformsNum; j++) {
            if (strstr(clPlatform[j].name, preferredPlatform[i])) {
                selectedPlatform = &clPlatform[j];
                break;
            }
        }

        if (selectedPlatform != NULL) break;
    }

    struct openCLDevice *selectedDevice = NULL;
    for (int i = 0; i < selectedPlatform->deviceNums; ++i) {
        if (selectedPlatform->devices[i].type == deviceType) {
            selectedDevice = &selectedPlatform->devices[i];
            break;
        }
        if (selectedDevice != NULL) {
            break;
        }
    }


    // если не нашлась платформа то берем первую попавшуюся
    if (selectedDevice == NULL) {
        for (int i = 0; i < platformsNum; ++i) {
            if (clPlatform[i].deviceNums > 0) {
                selectedDevice = &clPlatform[i].devices[0];
                break;
            }
        }
    }

    DEBUG_PRINT(printf("Selected device: %s\n", selectedDevice->name));

    end:
    return &(selectedDevice->id);
}
