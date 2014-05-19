#include <boost/python.hpp>
#include <boost/numpy.hpp>
#include <iostream>
#include <vector>
#include "opencl.h"
#include "opengl.h"
#include "base.h"

namespace bp = boost::python;
namespace np = boost::numpy;

class Wrapper
{
  public:
    Wrapper(const std::string & module) 
    {
        if (module == std::string("OpenCL")) {
            p = ImageProcessing::OpenCL::Create(ImageProcessing::OpenCL::GPU);
        } else if (module == std::string("OpenGL")){
            p = ImageProcessing::OpenGL::Create();
        }
    }

    ~Wrapper()
    {
        for(int i = 0; i < _arrayptrs.size(); ++i) {
            bp::decref(_arrayptrs[i]);
        }
    }

    bool Build(const std::string & code)
    {
        return p->Build(code);
    }

    bool Process()
    {
        return p->Process();
    }

    void AddInputBuffer(const np::ndarray & array)
    {
        _AddBuffer<true>(array);
    }

    void AddOutputBuffer(const np::ndarray & array)
    {
        _AddBuffer<false>(array);
    }

    void SetParameterInt(const std::string & name, int value)
    {
        p->SetParameter(name, value);
    }

    void SetParameterFloat(const std::string & name, float value)
    {
        p->SetParameter(name, value);
    }
    
  private:
    boost::shared_ptr<ImageProcessing::Base> p;
    std::vector<PyObject*> _arrayptrs;

    template<bool T_INPUT>
    void _AddBuffer(const np::ndarray & array)
    {
        _arrayptrs.push_back(array.ptr());
        bp::incref(array.ptr());
        ImageProcessing::Buffer b;
        b.width = array.shape(0);
        b.height = array.shape(1);
        b.channels = array.get_nd() == 3 ? array.shape(2) : 1;
        b.data = array.get_data();

        if (array.get_dtype() == np::dtype::get_builtin<double>()) {
            b.bytesPerPixel = 8;
        } else if (array.get_dtype() == np::dtype::get_builtin<float>()) {
            b.bytesPerPixel = 4;
        } else if (array.get_dtype() == np::dtype::get_builtin<int>()) {
            b.bytesPerPixel = 4;
        } else if (array.get_dtype() == np::dtype::get_builtin<unsigned int>()) {
            b.bytesPerPixel = 4;
        } else if (array.get_dtype() == np::dtype::get_builtin<unsigned char>()) {
            b.bytesPerPixel = 1;
        } else if (array.get_dtype() == np::dtype::get_builtin<char>()) {
            b.bytesPerPixel = 1;
        }
        b.bytesPerPixel *= b.channels;
        std::cout << "Bytes per pixel: " << b.bytesPerPixel << std::endl;
        
        if (T_INPUT) {
            p->AddInputBuffer(b);
        } else {
            p->AddOutputBuffer(b);
        }
    }
    
};

BOOST_PYTHON_MODULE(libyay)
{
    np::initialize();
    bp::class_<Wrapper>("Wrapper", bp::init<const std::string&>())
            .def("Build", &Wrapper::Build)
            .def("Process", &Wrapper::Process)
            .def("SetParameterInt", &Wrapper::SetParameterInt)
            .def("SetParameterFloat", &Wrapper::SetParameterFloat)
            .def("AddInputBuffer", &Wrapper::AddInputBuffer)
            .def("AddOutputBuffer", &Wrapper::AddOutputBuffer);
}
