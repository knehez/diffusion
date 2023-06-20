#define __CL_ENABLE_EXCEPTIONS
#include <iostream>
#include <fstream>
#include <vector>
#include <CL/opencl.hpp>
#include <chrono>
#include <thread>
#include <filesystem>

static std::string findFile(std::string fileName)
{
    for (auto &dirEntry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
    {
        if (dirEntry.is_regular_file())
        {
            auto file = dirEntry.path();
            if (file.filename().string() == fileName)
            {
                return file.string();
            }
        }
    }
    // file not found, return the original name
    return fileName;
}

bool printError(const std::vector<cl::Device>& devices, const cl::Program& program, const cl::Error& e)
{
	if (e.err() == CL_BUILD_PROGRAM_FAILURE)
	{
		for (cl::Device dev : devices)
		{
			// Check the build status
			const cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
			if (status != CL_BUILD_ERROR)
				continue;

			// Get the build log
			const auto name = dev.getInfo<CL_DEVICE_NAME>();
			const auto buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
			std::cerr << "Build log for " << name << ":" << std::endl
				<< buildLog << std::endl;
		}
	}
	return false;
}

int main()
{
    const int size = 10;

    std::vector<int> v1(size, 0);

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    const cl::Context context(CL_DEVICE_TYPE_GPU);
    const auto devices = context.getInfo<CL_CONTEXT_DEVICES>();

    std::ifstream kernelSource(findFile("kernel.cl"));

    if (kernelSource.fail())
    {
        std::cout << "Current path is " << std::filesystem::current_path() << '\n';
        std::cout << "kernel.cl: file not found\n";
        return 1;
    }
    const std::string sourceCode(std::istreambuf_iterator<char>{kernelSource}, {});

    cl::Program::Sources sources;
    sources.push_back(sourceCode);

    const cl::Program program(context, sources);
    
    try
    {
        program.build(devices);
    }
    catch (cl::Error &e)
    {
	    printError(devices, program, e);
        return -1;
    }


    cl::Kernel kernel(program, "vectorAdd");
    const cl::Buffer bufferV1(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * size, v1.data());

    kernel.setArg(0, bufferV1);

    const cl::CommandQueue queue(context, devices[0]);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size));
    queue.enqueueReadBuffer(bufferV1, CL_TRUE, 0, sizeof(int) * size, v1.data());

    std::cout << "v1: ";
    for (int i = 0; i < size; i++)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
