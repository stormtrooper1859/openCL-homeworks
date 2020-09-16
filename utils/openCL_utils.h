#ifndef OPENCL_HOMEWORKS_OPENCL_UTILS_H
#define OPENCL_HOMEWORKS_OPENCL_UTILS_H

#include <CL/cl.h>

struct openCLDevice {
    cl_device_id id;
    char *name;
    cl_device_type type;
};

struct openCLPlatform {
    cl_platform_id id;
    char *name;
    cl_uint deviceNums;
    struct openCLDevice *devices;
};

cl_device_id *getPreferredDevice(cl_device_type deviceType);

#endif //OPENCL_HOMEWORKS_OPENCL_UTILS_H
