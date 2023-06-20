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

int main() {
    // Vektor mérete
    const int size = 10;

    // Vektorok inicializálása
    std::vector<float> v1(size, 1.0f);
    std::vector<float> v2(size, 1.0f);

    // OpenCL inicializálása
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Context context(CL_DEVICE_TYPE_GPU);
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
    
    std::ifstream kernelSource(findFile("kernel.cl"));
    
    if(kernelSource.fail()){
        std::cout << "Current path is " << std::filesystem::current_path() << '\n';
        std::cout << "kernel.cl: file not found\n";
        return 1;
    }
    std::string sourceCode(std::istreambuf_iterator<char>{kernelSource}, {});

    cl::Program::Sources sources;
    sources.push_back(sourceCode);

    cl::Program program(context, sources);
    program.build(devices);

    // OpenCL kernel létrehozása
    cl::Kernel kernel1(program, "vectorAdd");
    cl::Kernel kernel2(program, "vectorAdd");

    // OpenCL buffer-ek létrehozása és másolása

    cl::Buffer bufferV1(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                        sizeof(int) * size, v1.data());

    cl::Buffer bufferV2(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                        sizeof(int) * size, v2.data());

    // Kernel1 paraméterek beállítása
    kernel1.setArg(0, bufferV1);
    kernel1.setArg(1, bufferV2);

    // Kernel1 paraméterek beállítása
    kernel2.setArg(0, bufferV2);
    kernel2.setArg(1, bufferV1);

    // OpenCL parancssor létrehozása
    cl::CommandQueue queue(context, devices[0]);

    // Iterációk futtatása
    for (int i = 0; i < 100; i++) {
        // Kernel futtatása
        queue.enqueueNDRangeKernel(i%2==0 ? kernel1 : kernel2, cl::NullRange, cl::NDRange(size));
        // Eredmény vektor másolása v1-be a következő iterációhoz
        queue.enqueueReadBuffer(i%2==0 ? bufferV2 : bufferV1, CL_TRUE, 0, sizeof(int) * size, v1.data());
    }

    // Eredmény kiíratása
    std::cout << "v1: ";

    for (int i = 0; i < size; i++) {
        std::cout << v1[i] << " ";
    }

    std::cout << std::endl;

    return 0;
}