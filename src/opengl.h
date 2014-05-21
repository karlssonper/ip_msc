#ifndef OPENGL_H_
#define OPENGL_H_

#include "base.h"
#include <boost/shared_ptr.hpp>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

namespace ImageProcessing {


class OpenGL : public Base
{
  public:
    virtual ~OpenGL();

    static boost::shared_ptr<OpenGL> Create(bool glContext)
    {
        return boost::shared_ptr<OpenGL>(new OpenGL(glContext));
    }

    virtual bool Build(const std::string & code);
    
    virtual bool Process();

  private:
    OpenGL(bool glContext);

    GLuint _shader;
    GLuint _vbo;
    GLuint _fb;
    GLuint _width;
    GLuint _height;
    std::vector<GLuint> _inputTextures;
    std::vector<GLuint> _outputTextures;
};

} // end namespace

#endif
