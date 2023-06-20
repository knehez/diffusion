#include "CLHelper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>
using std::cerr;
using std::endl;


#define MAX_TEXT_LENGTH 255
void printFloatConfig(cl_device_fp_config config);

bool getFirstPlatform(cl_platform_id& platform) {
	cl_uint count;
	clGetPlatformIDs(1, &platform, &count);
	if (!count) {
		cerr << "No platform installed!" << endl;
		return false;
	}
	return true;
}

bool getFirstGPUorAnyDevice(cl_platform_id platform, cl_device_id& device, bool &float64support) {
	cl_uint count;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &count);
	if (!count) {
		clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &count);
		if (!count) {
			cerr << "No device found!" << endl;
			return false;
		}
		cerr << "No GPU found!" << endl;
	}
	cerr << "Device being used:" << endl;
	char name[MAX_TEXT_LENGTH] = "<NA>", vendor[MAX_TEXT_LENGTH] = "<NA>", version[MAX_TEXT_LENGTH] = "<NA>";
	cl_device_type type; cl_device_fp_config single_config, double_config;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
	clGetDeviceInfo(device, CL_DEVICE_NAME, MAX_TEXT_LENGTH, name, NULL);
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, MAX_TEXT_LENGTH, vendor, NULL);
	clGetDeviceInfo(device, CL_DEVICE_VERSION, MAX_TEXT_LENGTH, version, NULL);
	clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &single_config, NULL);
	clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config), &double_config, NULL);
	float64support = double_config ? true : false;
	cerr << "\tname:\t\t" << name << endl;
	cerr << "\ttype:\t\t";
	if (type & CL_DEVICE_TYPE_DEFAULT) cerr << "DEFAULT ";
	if (type & CL_DEVICE_TYPE_CPU) cerr << "CPU ";
	if (type & CL_DEVICE_TYPE_GPU) cerr << "GPU ";
	if (type & CL_DEVICE_TYPE_ACCELERATOR) cerr << "ACCELERATOR ";
	if (type & CL_DEVICE_TYPE_CUSTOM) cerr << "CUSTOM ";
	cerr << endl;
	cerr << "\tvendor:\t\t" << vendor << endl;
	cerr << "\tversion:\t" << version << endl;
	cerr << "\tsingle support:" << endl;
	printFloatConfig(single_config);
	cerr << "\tdouble suppport:" << endl;
	printFloatConfig(double_config);
	cerr << endl;
	return true;
}

void printFloatConfig(cl_device_fp_config config) {
	if (!config) cerr << "\t\tNO SUPPORT" << endl;
	if (config & CL_FP_DENORM) cerr << "\t\tDENORM" << endl;
	if (config & CL_FP_INF_NAN) cerr << "\t\tINF NAN" << endl;
	if (config & CL_FP_ROUND_TO_NEAREST) cerr << "\t\tROUND TO NEAREST" << endl;
	if (config & CL_FP_ROUND_TO_ZERO) cerr << "\t\tROUND TO ZERO" << endl;
	if (config & CL_FP_ROUND_TO_INF) cerr << "\t\tROUND TO INF" << endl;
	if (config & CL_FP_FMA) cerr << "\t\tFUSED MULTIPLE ADD" << endl;
}

bool createContext(cl_device_id device, cl_context& context) {
	cl_int err;
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		printErr("creating context", err);
		return false;
	}
	return true;
}

bool createQueue(cl_device_id device, cl_context context, cl_command_queue& queue) {
	cl_int err;
	queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
	if (err != CL_SUCCESS) {
		printErr("creating command-queue for the device", err);
		return false;
	}
	return true;
}

bool initCL(cl_platform_id& platform, cl_device_id& device, cl_context& context, cl_command_queue& queue, bool &float64support) {
	return getFirstPlatform(platform)
		&& getFirstGPUorAnyDevice(platform, device, float64support)
		&& createContext(device, context)
		&& createQueue(device, context, queue);
}

bool printBuildLog(cl_program, cl_device_id);

bool compileCLProgramFromString(cl_device_id device, cl_uint source_size, const char** source, cl_context context, cl_program& program) {
	cl_int err;
	program = clCreateProgramWithSource(context, source_size, source, NULL, &err);
	if (err != CL_SUCCESS) {
		printErr("creating Program object", err);
		return false;
	}
	err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	printBuildLog(program, device);
	if (err != CL_SUCCESS) {
		printErr("building kernel-program", err);
		return false;
	}	
	return true;
}

bool printBuildLog(cl_program program, cl_device_id device) {
	size_t log_size;
	if (clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size) != CL_SUCCESS)
		return false;
	char* build_log = new char[log_size];
	if (clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL) != CL_SUCCESS) {
		delete[] build_log;
		return false;
	}
	cerr << build_log << endl;
	delete[] build_log;
	return true;
}

bool compileCLFile(cl_device_id device, cl_context context, const char* filename, cl_program& program) {
	std::ifstream file(filename, std::ios_base::in);
	std::vector<std::string> lines;
	while (file.good()) {
		std::string line;
		std::getline(file, line);
		if (!file.eof()) line.append("\n");  // Is it platform-independent?
		// Egyáltalán ki a f@szom találta ki,hogy a clCreateProgramWithSource soronként külön tömbelembe szedve várja a forrást (char**),
		// de mégse tud különbséget tenni sorok között, ha nincs explicit a sorok végén a delimiter?!
		lines.push_back(line);
	}
	if (file.fail() || file.bad()) {
		cerr << "Error while reading file: " << filename << endl;
		return false;
	}
	std::vector<const char*> c_lines;
	c_lines.reserve(lines.size());
	for (const std::string &line : lines)
	{
		c_lines.push_back(line.c_str());
	}
	return compileCLProgramFromString(device, c_lines.size(), c_lines.data(), context, program);
}

bool createMemBuffCopyHost(cl_context context, size_t size, void* init_data, cl_mem& buf, bool devWriteEna, bool hostReadbackEna) {
	cl_mem_flags flags = CL_MEM_COPY_HOST_PTR
		| (devWriteEna ? CL_MEM_READ_WRITE : CL_MEM_READ_ONLY)
		| (hostReadbackEna ? CL_MEM_HOST_READ_ONLY : CL_MEM_HOST_NO_ACCESS);
	cl_int err;
	buf = clCreateBuffer(context, flags, size, init_data, &err);
	if (err != CL_SUCCESS) {
		printErr("creating memory buffer", err);
		return false;
	}
	return true;
}

bool createMemBuffFromDevToHost(cl_context context, size_t size, cl_mem& buf, bool devReadbackEna, bool hostWriteEna) {
	cl_mem_flags flags = devReadbackEna ? CL_MEM_READ_WRITE : CL_MEM_WRITE_ONLY;
	if (!hostWriteEna) flags = flags | CL_MEM_HOST_READ_ONLY;
	cl_int err;
	buf = clCreateBuffer(context, flags, size, NULL, &err);
	if (err != CL_SUCCESS) {
		printErr("creating memory buffer", err);
		return false;
	}
	return true;
}

bool enqueueCopyBuffer(cl_command_queue queue, cl_mem src, cl_mem dest, size_t size, size_t src_offs, size_t dest_offs) {
	cl_uint err = clEnqueueCopyBuffer(queue, src, dest, src_offs, dest_offs, size, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printErr("copying memory buffer", err);
		return false;
	}
	return true;
}

bool getKernelWithArgs(cl_program program, const char* name, cl_kernel& kernel, size_t argc, ...) {
	cl_int err;
	kernel = clCreateKernel(program, name, &err);
	if (err != CL_SUCCESS) {
		printErr("selecting Kernel for name", err);
		return false;
	}

	va_list args;
	va_start(args, argc);
	for (size_t n = 0; n < argc; ++n) {
		size_t arg_size = va_arg(args, size_t);
		const void* arg_ptr = va_arg(args, const void*);
		err = clSetKernelArg(kernel, n, arg_size, arg_ptr);
		if (err != CL_SUCCESS) {
			va_end(args);
			printErr("setting Kernel-argument", err);
			return false;
		}
	}
	va_end(args);
	return true;
}

bool enqueueKernel(cl_command_queue queue, cl_kernel kernel, cl_event& evnt,
		size_t dim, const size_t* offset, const size_t* n_cells) {
	cl_int err = clEnqueueNDRangeKernel(queue, kernel, dim, offset, n_cells, NULL, 0, NULL, &evnt);
	if (err != CL_SUCCESS) {
		printErr("enqueuing kernel", err);
		return false;
	}
	return true;
}

bool readResult(cl_command_queue queue, cl_mem buffer, size_t size, void* data) {
	cl_int err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size, data, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printErr("copying back result from device", err);
		return false;
	}
	return true;
}

bool getExecutionStatus(cl_event evnt, cl_int& status) {
	cl_int err = clGetEventInfo(evnt, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(status), &status, NULL);
	if (err != CL_SUCCESS) {
		printErr("querying execution status", err);
		return false;
	}
	return true;
}

void printErr(const char* operation, cl_int err) {
	cerr << "Error when " << operation << ". Code: " << err << endl;
	if (err == CL_INVALID_VALUE) cerr << "\tINVALID VALUE" << endl;  // invalid argument value given to CL API function

	// get device of platform
	if (err == CL_INVALID_PLATFORM) cerr << "\tINVALID PLATFORM" << endl;
	if (err == CL_DEVICE_NOT_FOUND) cerr << "\t" << "DEVICE NOT FOUND" << endl;  // if (count==0) instead
	
	// create context for devices
	if (err == CL_INVALID_PROPERTY) cerr << "\tINVALID PROPERTY" << endl;
	if (err == CL_INVALID_DEVICE) cerr << "\tINVALID DEVICE" << endl;
	if (err == CL_DEVICE_NOT_AVAILABLE) cerr << "\t" << "tDEVICE NOT AVAILABLE" << endl;
	
	// create and use command-queue for device
	if (err == CL_INVALID_CONTEXT) cerr << "\tINVALID CONTEXT" << endl;
	if (err == CL_INVALID_QUEUE_PROPERTIES) cerr << "\tINVALID QUEUE PROPERTIES" << endl;
	if (err == CL_INVALID_DEVICE_QUEUE) cerr << "\tINVALID DEVICE QUEUE OBJECT" << endl;  // both copying buffers and enqueuing kernels

	// create and build program
	if (err == CL_INVALID_BINARY) cerr << "\tINVALID BINARY" << endl;  // only clCreateProgramWithBinary()
	if (err == CL_INVALID_PROGRAM) cerr << "\tINVALID PROGRAM OBJECT" << endl;
	if (err == CL_COMPILER_NOT_AVAILABLE) cerr << "\t" << "COMPILER NOT AVAILABLE" << endl;  // clBuildProgram(), clCompileProgram()
	if (err == CL_LINKER_NOT_AVAILABLE) cerr << "\t" << "LINKER NOT AVAILABLE" << endl;  // clLinkProgram()
	if (err == CL_INVALID_BUILD_OPTIONS) cerr << "\tINVALID BUILD OPTIONS" << endl;  // clBuildProgram()
	if (err == CL_INVALID_COMPILER_OPTIONS) cerr << "\tINVALID COMPILER OPTIONS" << endl;  // clCompileProgram()
	if (err == CL_INVALID_LINKER_OPTIONS) cerr << "\tINVALID LINKER OPTIONS" << endl;  // clLinkProgram()
	if (err == CL_BUILD_PROGRAM_FAILURE) cerr << "\t" << "BUILD PROGRAM FAILURE" << endl;  // clBuildProgram()
	if (err == CL_COMPILE_PROGRAM_FAILURE) cerr << "\t" << "COMPILE PROGRAM FAILURE" << endl;  // clCompileProgram()
	if (err == CL_LINK_PROGRAM_FAILURE) cerr << "\t" << "LINK PROGRAM FAILURE" << endl;  // clLinkProgram()

	// memory buffers
	if (err == CL_INVALID_BUFFER_SIZE) cerr << "\tINVALID BUFFER SIZE" << endl;  // creation only
	if (err == CL_INVALID_HOST_PTR) cerr << "\tINVALID HOST PTR" << endl;  // creation only
	if (err == CL_MEM_OBJECT_ALLOCATION_FAILURE) cerr << "\t" << "MEM OBJECT ALLOCATION FAILURE" << endl;  // creation only
	if (err == CL_OUT_OF_HOST_MEMORY) cerr << "\t" << "OUT OF HOST MEMORY" << endl;
	if (err == CL_INVALID_MEM_OBJECT) cerr << "INVALID MEMORY OBJECT" << endl;  // copying only
	if (err == CL_MEM_COPY_OVERLAP) cerr << "\t" << "MEM COPY OVERLAP" << endl;  // copying only
	if (err == CL_OUT_OF_RESOURCES) cerr << "\t" << "OUT OF RESOURCES" << endl;  // copying only

	// get and parametrize kernel of program
	if (err == CL_INVALID_KERNEL_NAME) cerr << "\tKERNEL NAME NOT FOUND" << endl;
	if (err == CL_INVALID_KERNEL_DEFINITION) cerr << "\tKERNEL DEFINITION MISMATCH" << endl;  // when using multiple devices for context
	if (err == CL_INVALID_KERNEL) cerr << "\tINVALID KERNEL OBJECT" << endl;
	if (err == CL_INVALID_ARG_INDEX) cerr << "\tINVALID ARGUMENT INDEX" << endl;
	if (err == CL_INVALID_ARG_SIZE) cerr << "\tINVALID ARGUMENT SIZE" << endl;
	if (err == CL_INVALID_ARG_VALUE) cerr << "\tINVALID ARGUMENT VALUE" << endl;
	//if (err == CL_KERNEL_ARG_INFO_NOT_AVAILABLE) cerr << "\t" << "KERNEL ARG INFO NOT AVAILABLE" << endl;  // querying back

	// enqueue, runtime
	if (err == CL_INVALID_WORK_DIMENSION) cerr << "\tINVALID WORK DIMENSION" << endl;
	if (err == CL_INVALID_GLOBAL_WORK_SIZE) cerr << "\tINVALID GLOBAL WORK SIZE" << endl;
	if (err == CL_INVALID_GLOBAL_OFFSET) cerr << "\tINVALID GLOBAL OFFSET" << endl;
	if (err == CL_INVALID_WORK_GROUP_SIZE) cerr << "\tINVALID WORKGROUP SIZE" << endl;
	if (err == CL_INVALID_WORK_ITEM_SIZE) cerr << "\tINVALID WORKITEM SIZE" << endl;
	if (err == CL_INVALID_KERNEL_ARGS) cerr << "\tKERNEL ARGUMENTS NOT SPECIFIED" << endl;
	if (err == CL_INVALID_PROGRAM_EXECUTABLE) cerr << "\tINVALID PROGRAM EXECUTABLE" << endl;
}