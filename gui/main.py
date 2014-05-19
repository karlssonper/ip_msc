import sys
from PySide import QtGui, QtCore
import paramWidget
import glWidget

import scipy.misc
import numpy
from PIL import Image
import libyay

code = """
__kernel void ip_kernel( __global unsigned char * imgA,
                                 __global unsigned char * imgB,
                                 __global unsigned char * out_blend,
                                 float alpha,
                                 float red,
                                 float green,
                                 float blue)
{ 
    const int x = get_global_id(0); 
    const int y = get_global_id(1); 

    const int idx = 3*(x + y * 512);
    out_blend[idx] = red*(alpha * imgA[idx] + (1-alpha) * imgB[idx]);
    out_blend[idx+1] = green*(alpha * imgA[idx+1] + (1-alpha) * imgB[idx+1]);
    out_blend[idx+2] = blue*(alpha * imgA[idx+2] + (1-alpha) * imgB[idx+2]);
}
"""

class MainWindow(QtGui.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        

        self.textEditor = QtGui.QPlainTextEdit(self)
        self.textEditor.appendPlainText(code)
        self.setCentralWidget(self.textEditor)

        dock = QtGui.QDockWidget("Log", self)
        self.log = QtGui.QTextBrowser(dock)
        dock.setAllowedAreas(QtCore.Qt.BottomDockWidgetArea)
        dock.setWidget(self.log)
        self.addDockWidget(QtCore.Qt.BottomDockWidgetArea, dock)
        self.printLog("Creating GUI components")
    
    
        self.printLog("Creating OpenCL context")
        self.ip = libyay.Wrapper("OpenCL")
   
        self.printLog("Creating OpenGL preview..")
        dock = QtGui.QDockWidget("Preview", self)
        test = QtGui.QWidget(dock)
        test2 = QtGui.QVBoxLayout()
        self.gl = glWidget.GLWidget(test)
        
        test2.addWidget(self.gl)

        self.runBtn = QtGui.QPushButton("Run")
        self.buildBtn = QtGui.QPushButton("Build")
        self.runBtn.clicked.connect(self.runImageProcessing)
        self.buildBtn.clicked.connect(self.buildImageProcessing)
        self.interactiveCheckBox = QtGui.QCheckBox("Interactive")
        test2.addStretch(1)
        wid = QtGui.QWidget(test)
        test2.addWidget(wid)
        test3 = QtGui.QHBoxLayout()
        wid.setLayout(test3)
        test3.addWidget(self.buildBtn)
        test3.addWidget(self.runBtn)
        test3.addWidget(self.interactiveCheckBox)
        
        
        test.setLayout(test2)

        dock.setAllowedAreas(QtCore.Qt.LeftDockWidgetArea)
        dock.setWidget(test)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock)

        
        
        dock = QtGui.QDockWidget("Parameters", self)
        self.parametersWidget = paramWidget.ParametersWidget(dock, self.interactiveImageProcessing)
        self.parametersWidget.addInputBuffer("imgA")
        self.parametersWidget.addInputBuffer("imgB")
   
        self.parametersWidget.addOutputBuffer("out_blend")
     
        #parametersWidget.addParameter("test", 13.37, -5.0, 15.0)
        #parametersWidget.addParameter("depth", 0, -10,10, "int")
        self.parametersWidget.addParameter("alpha", 1.0, 0, 1.0)
        self.parametersWidget.addParameter("red", 1.0, 0, 1.0)
        self.parametersWidget.addParameter("green", 1.0, 0, 1.0)
        self.parametersWidget.addParameter("blue", 1.0, 0, 1.0)
        
        dock.setAllowedAreas(QtCore.Qt.RightDockWidgetArea)
        dock.setWidget(self.parametersWidget)        
        self.addDockWidget(QtCore.Qt.RightDockWidgetArea, dock)

        self.setWindowTitle("GPU framework for Image Processing")
        self.show()
        
        # temp
        self.move(2000,520)

    def runImageProcessing(self):
        from time import time
        t = time()

        for param in self.parametersWidget.params:
            if param.typename == "int":
                self.ip.SetParameterInt(param.name,int(eval(param.lineEdit.text())))
            else:
                self.ip.SetParameterFloat(param.name,float(eval(param.lineEdit.text())))


        self.ip.Process()
        dt = time()-t
        self.printLog("Running Image Processing with OpenCL... took "+str(dt)+" seconds")
        self.gl.setData(self.outdata)
        self.gl.update()
        
        filename = str(self.parametersWidget.outputBuffers[0].lineEdit.text())
        if filename != "":
            out = Image.fromarray(self.outdata.transpose(1,0,2))
            out.save(filename)

    def buildImageProcessing(self):
        self.printLog("Creating buffers")
        image = Image.open(str(self.parametersWidget.inputBuffers[0].lineEdit.text()))
        data = numpy.array(image).transpose((1,0,2)).copy(order="C")
        image = Image.open(str(self.parametersWidget.inputBuffers[1].lineEdit.text()))
        ape = numpy.array(image).transpose((1,0,2)).copy(order="C")
        self.outdata = numpy.zeros((data.shape[0], data.shape[1], data.shape[2]), 
                              data.dtype)
        
        self.ip.AddInputBuffer(data)
        self.ip.AddInputBuffer(ape)
        self.ip.AddOutputBuffer(self.outdata)


        from time import time
        t = time()
        self.printLog("Building OpenCL kernel")
        self.ip.Build(str(self.textEditor.toPlainText()))
        dt = time()-t
        self.printLog("Building Image Processing with OpenCL... took "+str(dt)+" seconds")
        if self.interactiveCheckBox.isChecked():
            self.runImageProcessing()

    def interactiveImageProcessing(self):
        if self.interactiveCheckBox.isChecked():
            self.runImageProcessing()
   
    def printLog(self,msg):
        from time import gmtime, strftime
        tt = str(strftime("[%Y-%m-%d %H:%M:%S]: ", gmtime()))
        self.log.append(tt + msg)
def main():
    app = QtGui.QApplication(sys.argv)
    mainwindow = MainWindow()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
