#include "base.h"

namespace ImageProcessing {

void Base::AddInputBuffer(const Buffer & buffer)
{
    _inputBuffers.push_back(buffer);
}
    
void Base::AddOutputBuffer(const Buffer & buffer)
{
    _outputBuffers.push_back(buffer);
}

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

}//end namespace
