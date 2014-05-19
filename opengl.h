#ifndef OPENGL_H_
#define OPENGL_H_

#include "base.h"
#include <boost/shared_ptr.hpp>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

namespace ImageProcessing {


class OpenGL : public Base
{
  public:
    virtual ~OpenGL();

    static boost::shared_ptr<OpenGL> Create()
    {
        return boost::shared_ptr<OpenGL>(new OpenGL());
    }

    virtual bool Build(const std::string & code);
    
    virtual bool Process();

  private:
    OpenGL();

};

} // end namespace

#endif
