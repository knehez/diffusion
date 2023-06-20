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
            std::filesystem::path file = dirEntry.path();
            if (file.filename().string() == fileName)
            {
                return file.string();
            }
        }
    }
    return fileName;
}

int main()
{
    // Vektor mérete
    const int size = 10;

    // Vektorok inicializálása
    std::vector<int> v1(size, 0);

    // OpenCL inicializálása
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Context context(CL_DEVICE_TYPE_GPU);
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    std::ifstream kernelSource(findFile("kernel.cl"));

    if (kernelSource.fail())
    {
        std::cout << "Current path is " << std::filesystem::current_path() << '\n';
        std::cout << "kernel.cl: file not found\n";
        return 1;
    }
    std::string sourceCode(std::istreambuf_iterator<char>{kernelSource}, {});

    cl::Program::Sources sources;
    sources.push_back(sourceCode);

    cl::Program program(context, sources);
    
    try
    {
        program.build(devices);
    }
    catch (cl::Error &e)
    {
        if (e.err() == CL_BUILD_PROGRAM_FAILURE)
        {
            for (cl::Device dev : devices)
            {
                // Check the build status
                cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
                if (status != CL_BUILD_ERROR)
                    continue;

                // Get the build log
                std::string name = dev.getInfo<CL_DEVICE_NAME>();
                std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
                std::cerr << "Build log for " << name << ":" << std::endl
                          << buildlog << std::endl;
            }
            return -1;
        }
    }

    // OpenCL kernel létrehozása
    cl::Kernel kernel(program, "vectorAdd");

    // OpenCL buffer-ek létrehozása és másolása
    cl::Buffer bufferV1(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                        sizeof(int) * size, v1.data());

    // Kernel paraméterek beállítása
    kernel.setArg(0, bufferV1);

    // OpenCL parancssor létrehozása
    cl::CommandQueue queue(context, devices[0]);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size));

    // Eredmény vektor másolása v1-be a következő iterációhoz
    queue.enqueueReadBuffer(bufferV1, CL_TRUE, 0, sizeof(int) * size, v1.data());

    // Eredmény kiíratása
    std::cout << "v1: ";
    for (int i = 0; i < size; i++)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}