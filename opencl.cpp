#include "opencl.h"

namespace ImageProcessing {

OpenCL::OpenCL(DeviceType t)
{
    cl_int err;
    
    // Get Platform ID
    cl_platform_id platform_id;
    err = clGetPlatformIDs(1, &platform_id, NULL);
    
    if (err != CL_SUCCESS){
        throw std::logic_error("OpenCL() could not get platform id");
    }
    
    // Get Device type
    cl_device_type device_type;
    switch (t) {
        case GPU:
            device_type = CL_DEVICE_TYPE_GPU;
            break;
        case CPU:
            device_type = CL_DEVICE_TYPE_CPU;
            break;
        case BEST_FIT:
            //todo. For now always pick CPU. Let query decide
            device_type = CL_DEVICE_TYPE_CPU;
            break;
    }

    // Get Device ID
    err = clGetDeviceIDs(platform_id, device_type,
                         1 /*only 1 device for now */,
                         &_device_id, NULL);

    if (err != CL_SUCCESS){
        std::cerr << err << std::endl;
        throw std::logic_error("OpenCL() could not get device id");
    }
   
    // Create context and command queue
    _ctx = clCreateContext(NULL, 1, &_device_id, NULL, NULL, NULL);
    _queue = clCreateCommandQueue(_ctx, _device_id, 0, NULL);
}

OpenCL::~OpenCL()
{
   
}

bool OpenCL::Build(const std::string & code)
{
    // Create program
    const char * code_c_str = code.c_str();
    cl_program program = clCreateProgramWithSource(
        _ctx, 1, &code_c_str, NULL, NULL);

    // Build program
    cl_int err = clBuildProgram(program, 1, &_device_id, NULL, NULL, NULL);

    // Make sure program was built correctly
    if (err != CL_SUCCESS) {
        std::cerr << "OpenCL::Build failed: " << err << "\n";

        char buf[0x10000];
        clGetProgramBuildInfo(program, _device_id, CL_PROGRAM_BUILD_LOG,
                               0x10000, buf, NULL);
        std::cerr << buf << std::endl;
        return false;
    }

    // Create kernel from program
    _kernel = clCreateKernel(program, "kernel", NULL);

    // Create input Buffers on the device and copy data from host
    // Todo: Might be better to explore Pinned memory here
    _device_input_buffers.reserve(_inputBuffers.size());
    for (size_t i = 0; i < _inputBuffers.size(); ++i) {
        const Buffer & b = _inputBuffers[i];
        _device_input_buffers.push_back(
            clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           b.width*b.height*b.bytesPerPixel, b.data, NULL));
    }

    // Create output Buffers on the device
    _device_output_buffers.reserve(_outputBuffers.size());
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        const Buffer & b = _outputBuffers[i];
        _device_output_buffers.push_back(
            clCreateBuffer(_ctx, CL_MEM_WRITE_ONLY,
                           b.width*b.height*b.bytesPerPixel, b.data, NULL));
    }
    
    return true;
}

template<typename T>
void _SetParametersAsKernelArg(const std::vector<std::pair<std::string,T> > & v,
                               const cl_kernel & kernel,
                               cl_int & argc)
{
    for (size_t i = 0; i < v.size(); ++i) {
        clSetKernelArg(kernel, argc++, sizeof(T), &v[i].second);
    }
}

bool OpenCL::Process()
{
    // Set kernel arguments in the order:
    // Input buffers -> output buffers -> int parameters -> float parameters
    cl_int argc = 0;
    const size_t size = sizeof(cl_mem);
    for (size_t i = 0; i < _device_input_buffers.size(); ++i) {
        clSetKernelArg(_kernel, argc++, size, &_device_input_buffers[i]);
    }

    for (size_t i = 0; i < _device_output_buffers.size(); ++i) {
        clSetKernelArg(_kernel, argc++, size, &_device_output_buffers[i]);
    }
    
    _SetParametersAsKernelArg(_GetParametersVector<int>(), _kernel, argc);
    _SetParametersAsKernelArg(_GetParametersVector<float>(), _kernel, argc);

    // Execute kernel
    const size_t global_work_size[] = { _outputBuffers.front().width,
                                        _outputBuffers.front().height };
    clEnqueueNDRangeKernel(_queue, _kernel, 2, NULL,
                           global_work_size, NULL, 0, NULL, NULL);
    
    // Copy data from device to host
    for (size_t i = 0; i < _device_output_buffers.size(); ++i) {
        clEnqueueReadBuffer(_queue, _device_output_buffers[i], CL_TRUE,
                            0, 0, _outputBuffers[i].data, 0, NULL, NULL);
    }
   
    return true;
}

} //end namespace
