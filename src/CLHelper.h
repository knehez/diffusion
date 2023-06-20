#pragma once

#include <CL/opencl.h>


void printErr(const char* operation, cl_int err);
bool initCL(cl_platform_id&, cl_device_id&, cl_context&, cl_command_queue&, bool &float64support);
bool compileCLProgramFromString(cl_device_id device, cl_uint source_size, const char** source, cl_context context, cl_program& program);
bool compileCLFile(cl_device_id device, cl_context context, const char* filename, cl_program& program);
bool createMemBuffCopyHost(cl_context context, size_t size, void* init_data, cl_mem& buf, bool devWriteEna = true, bool hostReadbackEna = true);
bool createMemBuffFromDevToHost(cl_context context, size_t size, cl_mem& buf, bool devReadbackEna = true, bool hostWriteEna = false);
bool enqueueCopyBuffer(cl_command_queue queue, cl_mem src, cl_mem dest, size_t size, size_t src_offs = 0, size_t dest_offs = 0);
bool getKernelWithArgs(cl_program program, const char* name, cl_kernel& kernel, size_t argc, ...);
bool enqueueKernel(cl_command_queue, cl_kernel, cl_event&, size_t dim, const size_t *offset, const size_t *n_cells);
bool getExecutionStatus(cl_event evnt, cl_int&);
bool readResult(cl_command_queue queue, cl_mem buffer, size_t size, void* data);