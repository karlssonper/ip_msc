#ifndef CUDA_H_
#define CUDA_H_

#include "base.h"
#include <boost/shared_ptr.hpp>

#include <cuda.h>
#include <cuda_runtime.h>

namespace ImageProcessing {

class CCUDA : public Base
{
  public:
    virtual ~CCUDA();
    
    static boost::shared_ptr<CCUDA> Create()
    {
        return boost::shared_ptr<CCUDA>(new CCUDA());
    }

    virtual bool Build(const std::string & code);
    
    virtual bool Process();
    
  private:
    CCUDA();

    CUmodule _module;
    CUfunction _function;
    
    std::vector<float*> _device_input_buffers;
    std::vector<float*> _device_output_buffers;
};

}//end namespace

#endif
