#include "opengl.h"
#include <stdio.h>
#include <string.h>
#include <sstream>

namespace ImageProcessing {

const char * vert_shader_code = 
"attribute vec2 positionIn;"
"varying vec2 texcoord;"
"void main()"
"{"
"      gl_Position=vec4(vec2(-1) + 2*positionIn,0, 1);"
"      texcoord = positionIn;"
"}";

/*
OpenGL * g = NULL;

void reshape(int w, int h)
{
    glViewport(0,0,w,h);
}

void draw()
{
    g->Process();

    glutSwapBuffers();
}
*/

OpenGL::OpenGL(bool glContext)
{
    if (!glContext) {
        int argc = 1;
        char * argv = "";
        glutInit(&argc, &argv);
        glutInitDisplayMode(GLUT_DOUBLE);
        glutInitWindowPosition(0,0);
        glutInitWindowSize(1,1);
        glutCreateWindow("");
        glewInit();

        //g = this;
        //glutReshapeFunc(reshape);
        //glutDisplayFunc(draw);
    }
}

OpenGL::~OpenGL()
{
}

struct _QuadVertex { float pos[3]; float texCoords[2];};


inline GLenum _GetType(const Buffer & b)
{
    switch(b.bytesPerPixel / b.channels) {
        case 1:
            return GL_UNSIGNED_BYTE;
        case 4:
            return GL_FLOAT;
        default:
            std::cerr << "Unknown OpenGL data type.. " << std::endl;
            return GL_FLOAT;
    }
}

inline GLenum _GetChannels(const Buffer & b)
{
    switch(b.channels) {
        case 1:
            return GL_RED;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            std::cerr << "Unknown OpenGL buffer channels " << b.channels
                      << std::endl;
            return GL_RGB;
    }
}

bool _BuildShader(const std::string & code, GLuint & shader)
{
    //std::cout << "Building shaders" << std::endl;
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    int length = strlen(vert_shader_code);
    glShaderSource(vertexShaderID, 1, &vert_shader_code, &length);
    glCompileShader(vertexShaderID);

    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    const char * c = code.c_str();
    length = strlen(c);
    glShaderSource(fragShaderID, 1, &c, &length);
    glCompileShader(fragShaderID);
  
    shader = glCreateProgram();
    glAttachShader(shader, fragShaderID);
    glAttachShader(shader, vertexShaderID);
    glLinkProgram(shader);
  
    GLint err = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, &err);
    if (err == 0) {
        std::stringstream ss;
#define ERROR_BUFSIZE 1024
        GLchar errorLog[ERROR_BUFSIZE];
        GLsizei length;

        glGetShaderInfoLog(vertexShaderID, ERROR_BUFSIZE, &length, errorLog);
        ss << "Vertex shader errors:\n" << std::string(errorLog,length);

        glGetShaderInfoLog(fragShaderID, ERROR_BUFSIZE, &length, errorLog);
        ss << "\nFragment shader errors:\n" << std::string(errorLog,length);

        glGetShaderInfoLog(shader, ERROR_BUFSIZE, &length, errorLog);
        ss << "\nLinker errors:\n" << std::string(errorLog,length);

        std::cerr << ss.str();
        return false;
    }
    return true;
}

bool _BuildQuad(GLuint & vbo)
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = {0,0,1,0,1,1,0,1};
    glBufferData(GL_ARRAY_BUFFER, 32, vertices, GL_STATIC_DRAW);
    return true;
}

bool OpenGL::Build(const std::string & code)
{
    _BuildShader(code, _shader);
    _BuildQuad(_vbo);

    _width = _outputBuffers.front().width;
    _height = _outputBuffers.front().height;
    
    // Create input textures
    _inputTextures.resize(_inputBuffers.size());
    glGenTextures(_inputBuffers.size(), _inputTextures.data());
    for (size_t i = 0; i < _inputBuffers.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, _inputTextures[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap
        glTexImage2D(GL_TEXTURE_2D, 0, _GetChannels(_inputBuffers[i]),
                     _width, _height, 0,
                     _GetChannels(_inputBuffers[i]),
                     _GetType(_inputBuffers[i]), _inputBuffers[i].data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Create output textures
    _outputTextures.resize(_outputBuffers.size());
    glGenTextures(_outputBuffers.size(), _outputTextures.data());
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        if (_outputBuffers[i].width != _width or
            _outputBuffers[i].width != _width) {
            std::cerr << "OpenGL: Not all output buffers got the same dimension"
                      << std::endl;
            return false;
        }
        
        glBindTexture(GL_TEXTURE_2D, _outputTextures[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap
        glTexImage2D(GL_TEXTURE_2D, 0, _GetChannels(_outputBuffers[i]),
                     _width, _height, 0,
                     _GetChannels(_outputBuffers[i]),
                     _GetType(_outputBuffers[i]), 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
        
    // create a renderbuffer object to store depth info
    GLuint rboId;
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Create FBO
    glGenFramebuffers(1, &_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, _fb);

    // attach the texture to FBO color attachment point
    for (size_t i = 0; i < _outputBuffers.size(); ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER,          // 1. fbo target: GL_FRAMEBUFFER 
                               GL_COLOR_ATTACHMENT0+i,  // 2. attachment point
                               GL_TEXTURE_2D,           // 3. tex target: GL_TEXTURE_2D
                               _outputTextures[i],      // 4. tex ID
                               0);                      // 5. mipmap level: 0(base)
    }
   
        
    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
                              GL_DEPTH_ATTACHMENT, // 2. attachment point
                              GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
                              rboId);              // 4. rbo ID
    

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Could not create OpenGL Framebuffer object. " << std::endl;
        return false;
    }

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //glutMainLoop();
    
    return true;
}

bool OpenGL::Process()
{
   
    //std::cout << "Process" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, _fb);
    glViewport(0,0, _width, _height);
    
    
    std::vector<GLenum> enums;
    for (unsigned int i = 0; i < _outputBuffers.size(); ++i) {
        enums.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    
    glDrawBuffers(enums.size(), &enums[0]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shader);
     
    GLint loc = glGetAttribLocation(_shader, "positionIn");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, 0, 8, 0);
    
    for (size_t i = 0; i < _intParameters.size(); ++i) {
        loc = glGetUniformLocation(_shader,_intParameters[i].first.c_str());
        glUniform1i(loc, _intParameters[i].second);
    }
    for (size_t i = 0; i < _floatParameters.size(); ++i) {
        loc = glGetUniformLocation(_shader,_floatParameters[i].first.c_str());
        glUniform1f(loc, _floatParameters[i].second);
    }

    GLint active;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
    
    for(size_t i = 0; i < _inputTextures.size(); ++i) {
        std::stringstream ss;
        ss << "texture" << i;
        loc = glGetUniformLocation(_shader, ss.str().c_str());
        //std::cout << "texture location at: " << loc << std::endl;
        glUniform1i(loc, i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _inputTextures[i]);
    }
    
    glDrawArrays(GL_QUADS, 0, 4);
    glActiveTexture(active);
    for (unsigned int i = 0; i < _outputBuffers.size(); ++i) {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        glReadPixels(0, 0, _width, _height,
                     _GetChannels(_outputBuffers[i]),
                     _GetType(_outputBuffers[i]), _outputBuffers[i].data);
    }
    /*
  */
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0); 
     
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
   
    
    return true;
}

} // end namespace ImageProcessing
