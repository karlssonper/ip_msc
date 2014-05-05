#include "opencl.h"

using namespace ImageProcessing;

const char *my_program = "__kernel void test( __global float * a, __global float * b )  { int x = get_global_id(0); int y = get_global_id(1); b[x+y*5] = x;}";

int main()
{
    boost::shared_ptr<OpenCL> cl = OpenCL::Create(OpenCL::CPU);


    float * data = new float[25];

    data[15] = 15;
    
    Buffer input, output;
    input.width = 5;
    input.height = 5;
    input.channels = 1;
    input.bytesPerPixel = 4;
    input.data = data;
    output = input;

    cl->AddInputBuffer(input);
    cl->AddOutputBuffer(input);

    std::cerr << "we good" << std::endl;
    
    cl->Build(my_program);
    std::cerr << "we not good" << std::endl;
    
    cl->Process();

    std::cerr << "error" << std::endl;

    for (int i = 0; i < 25; ++i) {
        std::cout << data[i] << std::endl;
    }
    
    return 0;
}
