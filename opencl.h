#ifndef OPENCL_H_
#define OPENCL_H_

#include "base.h"
#include <boost/shared_ptr.hpp>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

namespace ImageProcessing {

class OpenCL : public Base
{
  public:
    virtual ~OpenCL();
    
    enum DeviceType { GPU, CPU, BEST_FIT };
    
    static boost::shared_ptr<OpenCL> Create(DeviceType t)
    {
        return boost::shared_ptr<OpenCL>(new OpenCL(t));
    }

    virtual bool Build(const std::string & code);
    
    virtual bool Process();
    
  private:
    OpenCL(DeviceType t);
    
    cl_device_id _device_id;
    cl_context _ctx;
    cl_command_queue _queue;
    cl_kernel _kernel;

    std::vector<cl_mem> _device_input_buffers;
    std::vector<cl_mem> _device_output_buffers;
};

}//end namespace

#endif
