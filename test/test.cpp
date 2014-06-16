#include "opencl.h"
#include "opengl.h"
#include "ccuda.h"
#include <cassert>

using namespace ImageProcessing;

const char * opencl_code = ""
"__kernel void ip_kernel(__global float * a,"
"                        __global float * bA,"
"                        __global float * bB,"
"                        int incA,"
"                        float incB)"
"{"
"    int x = get_global_id(0); int y = get_global_id(1);"
"    bA[x+y*5] =  a[x+y*5] + incA *0.1;"
"    bB[x+y*5] =  a[x+y*5] + incB ;"        
"}";

const char * cuda_code = ""
"__global__ void kernel(float *a, float * bA, float * bB, int incA, float incB)"
"{"
   "bA[blockIdx.x + 5*blockIdx.y] = a[blockIdx.x + 5*blockIdx.y] + incA*0.1;"
   "bB[blockIdx.x + 5*blockIdx.y] = a[blockIdx.x + 5*blockIdx.y] + incB;"
"}";

const char * opengl_code = ""
"uniform sampler2D texture0;"
"uniform int incA;"
"uniform float incB;"        
"varying vec2 texcoord;"
"void main()"
"{"
"    gl_FragData[0]= vec4(texture2D(texture0, texcoord).x+incA*0.1,0,0,1);"
"    gl_FragData[1]= vec4(texture2D(texture0, texcoord).x+incB,0,0,1);"
"}";

inline bool equal(float a, float b)
{
    return abs(a-b) < 0.001;
}

void test(boost::shared_ptr<Base> ip, const char * code)
{
    std::vector<float> data_input(5*5), data_outputA(5*5), data_outputB(5*5);
    for (int i = 0; i < 5*5; ++i) {
        data_input[i] = i/50.0;
    }
    const float incB = 0.25f;
    const int incA = 2.0;
    Buffer input, outputA, outputB;
    input.width = 5;
    input.height = 5;
    input.channels = 1;
    input.bytesPerPixel = 4;
    input.data = data_input.data();
    outputA= input;
    outputA.data = data_outputA.data();
    outputB = input;
    outputB.data = data_outputB.data();
    
    ip->AddInputBuffer(input);
    ip->AddOutputBuffer(outputA);
    ip->AddOutputBuffer(outputB);
    ip->SetParameter("incA", incA);
    ip->SetParameter("incB", incB);
    ip->Build(code);
    ip->Process();

    for (int i = 0; i < 5*5; ++i) {
        assert(equal(data_outputA[i], data_input[i] + incA*0.1) && "Per element inc");
        assert(equal(data_outputB[i], data_input[i] + incB) && "Per element inc");
    }
}

int main()
{
    std::cout << "Testing: OpenCL..." << std::endl;
    boost::shared_ptr<OpenCL> cl = OpenCL::Create(OpenCL::GPU);
    test(cl, opencl_code);
    std::cout << "Test passed: OpenCL.\n" << std::endl;

    std::cout << "Testing: CUDA..." << std::endl;
    boost::shared_ptr<CCUDA> cuda = CCUDA::Create();
    test(cuda, cuda_code);
    std::cout << "Test passed: CUDA.\n" << std::endl;

    std::cout << "Testing: OpenGL..." << std::endl;
    boost::shared_ptr<OpenGL> gl = OpenGL::Create(false);
    test(gl, opengl_code);
    std::cout << "Test passed: OpenGL." << std::endl;
       
    return 0;
}
