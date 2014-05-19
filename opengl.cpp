#include "opengl.h"

namespace ImageProcessing {

const char * vert_shader_code = 
"void main()"
"{"
"      gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
"}";

const char * frag_shader_code = 
"void main()"
"{"
"    gl_FragColor=vec4(1,0,0,1);"
"}";

OpenGL::OpenGL()
{
    int argc = 1;
    char * argv = "";
    glutInit(&argc, &argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    //glutInitWindowPosition(0,0);
    //glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("hidden window");

    GLuint color_tex;
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    GLuint fb;
    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status == GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::cout << "Framebuffer ok!" << std::endl;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //-------------------------
    glViewport(0, 0, 256, 256);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 256.0, 0.0, 256.0, -1.0, 1.0); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //-------------------------
    glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(1,0);
    glVertex2f(1,1);
    glVertex2f(0,1);
    glEnd();
    //

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

OpenGL::~OpenGL()
{
}

bool OpenGL::Build(const std::string & code)
{
    return true;
}

bool OpenGL::Process()
{
    return true;
}

} // end namespace ImageProcessing
