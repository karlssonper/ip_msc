from PySide import QtGui, QtOpenGL, QtCore
from OpenGL import GL

class GLWidget(QtOpenGL.QGLWidget):
    def __init__(self, parent):
        super(GLWidget, self).__init__(parent)
    
        self.w,self.h = 200,200
        sp = QtGui.QSizePolicy()
        sp.setHeightForWidth(True)
        #self.setSizePolicy(sp)

        self.texture = None


    def setData(self, data):
        print data.shape, data.dtype
        self.w = data.shape[1]
        self.h = data.shape[0]
        self.updateGeometry()
        self.texture = GL.glGenTextures(1)
        GL.glBindTexture(GL.GL_TEXTURE_2D, self.texture)
        GL.glPixelStorei(GL.GL_UNPACK_ALIGNMENT,1)
        GL.glTexParameterf( GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_S, GL.GL_CLAMP )
        GL.glTexParameterf( GL.GL_TEXTURE_2D, GL.GL_TEXTURE_WRAP_T, GL.GL_CLAMP )
        GL.glTexParameterf( GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR )
        GL.glTexParameterf( GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR )
    
        GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, GL.GL_RGB, self.w, self.h, 0 , 
                               GL.GL_RGB, GL.GL_UNSIGNED_BYTE, data)

    def initializeGL(self):
        pass#self.setData(self.data)

    def paintGL(self):
        GL.glMatrixMode(GL.GL_MODELVIEW)
        GL.glLoadIdentity()

        GL.glClear(GL.GL_COLOR_BUFFER_BIT)

        GL.glEnable(GL.GL_TEXTURE_2D)
        if self.texture:
            GL.glBindTexture(GL.GL_TEXTURE_2D, self.texture)

        GL.glDisable(GL.GL_LIGHTING)

        GL.glColor3f(1,1,1)

        # Draw quad
        GL.glBegin(GL.GL_QUADS)
        GL.glTexCoord2f(1,0)
        GL.glVertex2f(0,0)
        GL.glTexCoord2f(1,1)
        GL.glVertex2f(1,0)
        GL.glTexCoord2f(0,1)
        GL.glVertex2f(1,1)
        GL.glTexCoord2f(0,0)
        GL.glVertex2f(0,1)
        GL.glEnd()

        GL.glDisable(GL.GL_TEXTURE_2D)

    def resizeGL(self, width, height):
        GL.glViewport(0,0,width,height)

        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadIdentity()
        GL.glOrtho(0,1,0,1,0,1)
        GL.glMatrixMode(GL.GL_MODELVIEW)

    def sizeHint(self):
        return QtCore.QSize(self.w,self.h)


