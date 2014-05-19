#include "opencl.h"

namespace ImageProcessing {

bool _CheckGetPlatformIdError(cl_int);
bool _CheckGetDeviceIdError(cl_int);
bool _CheckCreateProgram(cl_int);
bool _CheckBuildProgramError(cl_int, cl_program, cl_device_id);
bool _CheckCreateKernelError(cl_int);
bool _CheckCreateBufferError(cl_int);
bool _CheckEnqueueNDRangeKernelError(cl_int);
bool _CheckEnqueueReadBufferError(cl_int);

OpenCL::OpenCL(DeviceType t)
{
    cl_int err;
    
    // Get Platform ID
    cl_platform_id platform_id;
    err = clGetPlatformIDs(1, &platform_id, NULL);
    if (!_CheckGetPlatformIdError(err)) {
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
            //todo. For now always pick CPU. Let cpu/gpu query decide in future
            device_type = CL_DEVICE_TYPE_CPU;
            break;
    }

    // Get Device ID
    err = clGetDeviceIDs(platform_id, device_type,
                         1 /*only 1 device for now */,
                         &_device_id, NULL);
    if (!_CheckGetDeviceIdError(err)) {
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
    cl_int err;
    
    // Create program
    const char * code_c_str = code.c_str();
    cl_program program = clCreateProgramWithSource(
        _ctx, 1, &code_c_str, NULL,  &err);
    if (!_CheckCreateProgram(err)) {
        return false;
    }

    // Build program
    err = clBuildProgram(program, 1, &_device_id, NULL, NULL, NULL);
    if (!_CheckBuildProgramError(err, program, _device_id)) {
        return false;
    }

    // Create kernel from program
    _kernel = clCreateKernel(program, "ip_kernel", &err);
    if (!_CheckCreateKernelError(err)) {
        return false;
    }

    for (size_t i = 0; i < _device_input_buffers.size(); ++i) {
        clReleaseMemObject(_device_input_buffers[i]);
    }
    _device_input_buffers.clear();
    for (size_t i = 0; i < _device_output_buffers.size(); ++i) {
        clReleaseMemObject(_device_output_buffers[i]);
    }
    _device_output_buffers.clear();
    
    // Create input Buffers on the device and copy data from host
    // Todo: Might be better to explore Pinned memory here
    _device_input_buffers.reserve(_inputBuffers.size());
    for (size_t i = 0; i < _inputBuffers.size(); ++i) {
        const Buffer & b = _inputBuffers[i];
        _device_input_buffers.push_back(
            clCreateBuffer(_ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           b.width*b.height*b.bytesPerPixel, b.data, &err));
        _CheckCreateBufferError(err);
    }

    // Create output Buffers on the device
    _device_output_buffers.reserve(_outputBuffers.size());
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        const Buffer & b = _outputBuffers[i];
        _device_output_buffers.push_back(
            clCreateBuffer(_ctx, CL_MEM_WRITE_ONLY,
                           b.width*b.height*b.bytesPerPixel, NULL, &err));
        _CheckCreateBufferError(err);
    }
    
    return true;
}

template<typename T>
void _SetParametersAsKernelArg(const std::vector<std::pair<std::string,T> > & v,
                               const cl_kernel & kernel,
                               cl_int & argc)
{
    cl_int err;
    for (size_t i = 0; i < v.size(); ++i) {
        err = clSetKernelArg(kernel, argc++, sizeof(T), &v[i].second);
        if (err != CL_SUCCESS) {
            std::cout << "ERROR: "<< err << std::endl; // todo handle in sep function
        }
    }
}

bool OpenCL::Process()
{
    cl_int err;
    
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

     // Execute kernel, first output buffer dimensiosn decides the work size.
    const size_t global_work_size[] = { _outputBuffers.front().width,
                                        _outputBuffers.front().height };

    std::cout << "Running OpenCL kernel with global work size: "
              << global_work_size[0] << " x " << global_work_size[1] << std::endl;

    
    err = clEnqueueNDRangeKernel(_queue, _kernel, 2, NULL,
                           global_work_size, NULL, 0, NULL, NULL);
    _CheckEnqueueNDRangeKernelError(err);
    
    // Copy data from device to host
    for (size_t i = 0; i < _device_output_buffers.size(); ++i) {
        const Buffer & b = _outputBuffers[i];
        err = clEnqueueReadBuffer(_queue, _device_output_buffers[i], CL_TRUE,
                            0, b.width * b.height * b.bytesPerPixel, b.data, 0, NULL, NULL);
        _CheckEnqueueReadBufferError(err);
    }
    
    return true;
}

bool _CheckGetPlatformIdError(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Could not get OpenCL platform ID. Error: "
                    << err << std::endl;
        return false;
    }
}

bool _CheckGetDeviceIdError(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Could not get OpenCL device ID. Error:  "
                    << err << std::endl;
        return false;
    }
}

bool _CheckCreateProgram(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Could not create OpenCL program. Error:  "
                    << err << std::endl;
        return false;
    }
}

bool _CheckBuildProgramError(cl_int err, cl_program p, cl_device_id device_id)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Building OpenCL program failed. Error: " << err << "\n";

        char buf[0x10000];
        clGetProgramBuildInfo(p, device_id, CL_PROGRAM_BUILD_LOG,
                              0x10000, buf, NULL);
        std::cerr << buf << std::endl;
        return false;
    }
}

bool _CheckCreateKernelError(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Could not create OpenCL kernel. Error: "
                    << err << std::endl;
        return false;
    }
}


bool _CheckCreateBufferError(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Could not create OpenCL buffer. Error: "
                    << err << std::endl;
        return false;
    }
}
bool _CheckEnqueueNDRangeKernelError(cl_int err) 
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Error when enqueuing OpenCL kernel. Error: "
                  << err << std::endl;
        return false;
    }
}

bool _CheckEnqueueReadBufferError(cl_int err)
{
    if (err == CL_SUCCESS) {
        return true;
    } else {
        std::cerr << "Error when reading from an OpenCL buffer. Error: "
                  << err << std::endl;
        return false;
    }
}

} //end namespace
