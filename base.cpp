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

}//end namespace
