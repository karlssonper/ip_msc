#include "opencl.h"

using namespace ImageProcessing;

const char *my_program = "__kernel void foo( __global float * a, __global float * b )  { int x = get_global_id(0), y = get_global_id(1); b[x+5*y] = 3;}";


int main()
{
    boost::shared_ptr<OpenCL> cl = OpenCL::Create(OpenCL::GPU);


    float * data = new float[25];
    
    Buffer input, output;
    input.width = 5;
    input.height = 5;
    input.channels = 1;
    input.bytesPerPixel = 4;
    input.data = data;
    output = input;

    cl->AddInputBuffer(input);
    cl->AddOutputBuffer(output);
    
    cl->Build(my_program);
    cl->Process();

    for (int i = 0; i < 25; ++i) {
        std::cout << data[i] << std::endl;
    }
    
    return 0;
}
