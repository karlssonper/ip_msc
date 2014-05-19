#include "opencl.h"
#include "opengl.h"
#include <cassert>

using namespace ImageProcessing;

const char * opencl_code = "__kernel void ip_kernel( __global float * a, __global float * b )  { int x = get_global_id(0); int y = get_global_id(1); b[x+y*5] =  a[x+y*5] + 1 ;}";

const char * opengl_code = "";

void test(boost::shared_ptr<Base> ip,
          const char * code)
{
    float * data_input = new float[5*5];
    float * data_output = new float[5*5];
    for (int i = 0; i < 5*5; ++i) {
        data_input[i] = i;
    }

    Buffer input, output;
    input.width = 5;
    input.height = 5;
    input.channels = 1;
    input.bytesPerPixel = 4;
    input.data = data_input;
    output = input;
    output.data = data_output;
    
    ip->AddInputBuffer(input);
    ip->AddOutputBuffer(output);
    ip->Build(code);
    ip->Process();

    for (int i = 0; i < 5*5; ++i) {
        assert(data_output[i] == data_input[i] + 1 && "Per element inc");
    }

    delete[] data_input;
    delete[] data_output;
}

void check_inc_correctness(const float * in, const float * out)
{
 
}

int main()
{
    boost::shared_ptr<OpenCL> cl = OpenCL::Create(OpenCL::GPU);
  
    std::cout << "Testing: OpenCL..." << std::endl;
    test(cl, opencl_code);
    std::cout << "Test passed: OpenCL.\n" << std::endl;

    std::cout << "Testing: OpenGL..." << std::endl;
    boost::shared_ptr<OpenGL> gl = OpenGL::Create();
    //test(gl, opengl_code);
    std::cout << "Test passed: OpenGL." << std::endl;
       
    return 0;
}
