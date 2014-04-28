#ifndef BASE_H_
#define BASE_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <stdexcept>

#include "types.h"

namespace ImageProcessing {

struct Buffer
{
    unsigned int width;
    unsigned int height;
    unsigned int channels;
    unsigned int bytesPerPixel;
    void * data;
};

class Base
{
  public:
    virtual ~Base() {}

    void AddInputBuffer(const Buffer &);
    
    void AddOutputBuffer(const Buffer &);

    template<typename T>
    void SetParameter(const std::string & name, const T & parameter);
    
    virtual bool Build(const std::string & code) = 0;
    
    virtual bool Process() = 0;

  protected:
    Base() {}

    std::vector<Buffer> _inputBuffers;
    std::vector<Buffer> _outputBuffers;

    std::vector<std::pair<std::string, int> >_intParameters;
    std::vector<std::pair<std::string, float> > _floatParameters;

    template<typename T>
    std::vector<std::pair<std::string, T> > & _GetParametersVector();
};

template<typename T>
void Base::SetParameter(const std::string & name, const T & value)
{
    std::vector<std::pair<std::string, T> > & v = _GetParametersVector<T>();

    // If name already in parameters vector, just update the value
    bool found = false;
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i].first == name) {
            v[i].second = value;
        }
    }

    // If name not in the parameters vector, put it at the back
    if (!found) {
        v.push_back(make_pair(name, value));
    }
    
}

template<typename T>
std::vector<std::pair<std::string, T> > & Base::_GetParametersVector()
{
    /* throw std::invalid_argument(std::string("Parameter of type ") +
                                std::string(typeid(T).name()) +
                                std::string(" not yet supported."));
    */
}
/*
template<>
std::vector<std::pair<std::string,int> > & Base::_GetParametersVector<int>()
{
    return _intParameters;
}


template<>
std::vector<std::pair<std::string,float> > & Base::_GetParametersVector<float>()
{
    return _floatParameters;
}
*/


} //end namespace

#endif
