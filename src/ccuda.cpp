#include "ccuda.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
namespace ImageProcessing {

inline int gpuGetMaxGflopsDeviceId()
{
	int device_count = 0;
	cudaGetDeviceCount( &device_count );

	cudaDeviceProp device_properties;
	int max_gflops_device = 0;
	int max_gflops = 0;
	
	int current_device = 0;
	cudaGetDeviceProperties( &device_properties, current_device );
	max_gflops = device_properties.multiProcessorCount * device_properties.clockRate;
	++current_device;

	while( current_device < device_count )
	{
		cudaGetDeviceProperties( &device_properties, current_device );
		int gflops = device_properties.multiProcessorCount * device_properties.clockRate;
		if( gflops > max_gflops )
		{
			max_gflops        = gflops;
			max_gflops_device = current_device;
		}
		++current_device;
	}

	return max_gflops_device;
}


CCUDA::CCUDA()
{
    int cuda_device = gpuGetMaxGflopsDeviceId();
    cudaSetDevice(cuda_device);
}

CCUDA::~CCUDA()
{
    
}

bool CCUDA::Build(const std::string & code)
{
    int err;

    for (size_t i = 0; i < _device_input_buffers.size(); ++i) {
        cudaFree(_device_input_buffers[i]);
    }
    _device_input_buffers.clear();
    for (size_t i = 0; i < _device_output_buffers.size(); ++i) {
        cudaFree(_device_output_buffers[i]);
    }
    _device_output_buffers.clear();
    
    // Create input Buffers on the device and copy data from host
    // Todo: Might be better to explore Pinned memory here
    _device_input_buffers.reserve(_inputBuffers.size());
    for (size_t i = 0; i < _inputBuffers.size(); ++i) {
        const Buffer & b = _inputBuffers[i];
        _device_input_buffers.push_back(new float);
        const size_t memsize = b.width*b.height*b.bytesPerPixel;
        err = cudaMalloc(&_device_input_buffers[i], memsize);
        //std::cout << err << std::endl;
        err = cudaMemcpy(_device_input_buffers[i], b.data, 
                   memsize, cudaMemcpyHostToDevice);
        //std::cout << err << std::endl;
    }

    // Create output Buffers on the device
    _device_output_buffers.reserve(_outputBuffers.size());
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        const Buffer & b = _outputBuffers[i];
        _device_output_buffers.push_back(new float);
        const size_t memsize = b.width*b.height*b.bytesPerPixel;
        //std::cout << err << " " << _device_output_buffers[i] <<  std::endl;
        err = cudaMalloc(&_device_output_buffers[i], memsize);
        //std::cout << err << " " << _device_output_buffers[i] <<  std::endl;
    }
    
    std::ofstream out(".temp.cu");
    out << "extern \"C\" { " << code << "}";
    out.close();

    int nvcc_exit_status = system("nvcc -ptx .temp.cu -o .temp.ptx");

    if (nvcc_exit_status) {
        std::cerr << "Error, could not compile" << std::endl;
        return false;
    }

    system("rm .temp.cu");

    
    //std::cout << "Loading from .temp.ptx" << std::endl;
    err = cuModuleLoad(&_module, ".temp.ptx");
    //std::cout << err << std::endl;

    system("rm .temp.ptx");

    
    //std::cout << "Getting function called 'kernel'" << std::endl;
    //err = cuModuleGetFunction(&_function, _module, "_Z6kernelPfS_");
    err = cuModuleGetFunction(&_function, _module, "kernel");
    //std::cout << err << std::endl;
}
    
bool CCUDA::Process()
{
    int err;
    cuFuncSetBlockShape(_function, 1, 1, 1);

    int paramOffset = 0;
    for (size_t i = 0; i < _inputBuffers.size(); ++i) {
        err = cuParamSetv(_function, paramOffset, &_device_input_buffers[i], sizeof(void*));
        paramOffset += sizeof(void *);
    }
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        cuParamSetv(_function, paramOffset, &_device_output_buffers[i], sizeof(void*));
        paramOffset += sizeof(void *);
    }

    for (size_t i = 0; i < _intParameters.size(); ++i) {
        cuParamSetv(_function, paramOffset, &_intParameters[i].second, sizeof(int));
        paramOffset += sizeof(int);
    }
    
    for (size_t i = 0; i < _floatParameters.size(); ++i) {
        cuParamSetv(_function, paramOffset, &_floatParameters[i].second, sizeof(float));
        paramOffset += sizeof(float);
    }

    cuParamSetSize(_function, paramOffset);
    
    err = cuLaunchGrid(_function, 
                       _inputBuffers.front().width,
                       _inputBuffers.front().height);
    
    //std::cout << "kernel " << err << std::endl;
    
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        Buffer & b = _outputBuffers[i];
        const size_t memsize = b.width*b.height*b.bytesPerPixel;
        err = cudaMemcpy( b.data, _device_output_buffers[i],
                   memsize, cudaMemcpyDeviceToHost);
    }
    //std::cout << "mem cpy " << err << std::endl;
    
}

} //end Namespace
