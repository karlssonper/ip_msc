#!/usr/bin/env python
import sys
from PySide import QtGui, QtCore
import paramWidget
import glWidget
import darkorange_stylesheet
import numpy
from PIL import Image
import PyIp

wrapper_type = None

class MainWindow(QtGui.QMainWindow):
    def __init__(self, ip_file):
        super(MainWindow, self).__init__()
        self.textEditor = QtGui.QPlainTextEdit(self)
        self.setCentralWidget(self.textEditor)

        dock = QtGui.QDockWidget("Log", self)
        self.log = QtGui.QTextBrowser(dock)
        dock.setAllowedAreas(QtCore.Qt.BottomDockWidgetArea)
        dock.setWidget(self.log)
        self.addDockWidget(QtCore.Qt.BottomDockWidgetArea, dock)
        self.printLog("Creating GUI components")
    
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

        dock.setAllowedAreas(QtCore.Qt.RightDockWidgetArea)
        dock.setWidget(self.parametersWidget)        
        self.addDockWidget(QtCore.Qt.RightDockWidgetArea, dock)

        self.setWindowTitle("GPU framework for Image Processing")
        
        
        self.createActions()
        self.createMenus()

        if ip_file != "":
            self.openImageProcessing(ip_file)

        # temp
        self.setGeometry(0,0,1200,440)
        #self.move(2000,520)

    def createActions(self):
        self.newAct = QtGui.QAction(QtGui.QIcon(""), "&New", self)
        self.newAct.setShortcut(QtGui.QKeySequence.New)
        self.newAct.triggered.connect(self.newImageProcessing)

        self.openAct = QtGui.QAction(QtGui.QIcon(""), "&Open", self)
        self.openAct.setShortcut(QtGui.QKeySequence.Open)
        self.openAct.triggered.connect(self.openImageProcessing)

        self.saveAct = QtGui.QAction(QtGui.QIcon(""), "&Save As", self)
        self.saveAct.setShortcut(QtGui.QKeySequence.Save)
        self.saveAct.triggered.connect(self.saveImageProcessing)

        self.quitAct = QtGui.QAction("&Quit", self)
        self.quitAct.setShortcut("Ctrl+Q")
        self.quitAct.triggered.connect(self.close)

        self.runAct = QtGui.QAction("&Run Image Processing", self)
        self.runAct.setShortcut("Ctrl+R")
        self.runAct.triggered.connect(self.runImageProcessing)

        self.buildAct = QtGui.QAction("&Build Image Processing", self)
        self.buildAct.setShortcut("Ctrl+B")
        self.buildAct.triggered.connect(self.buildImageProcessing)

        self.aboutQtAct = QtGui.QAction("About &Qt", self)
        self.aboutQtAct.triggered.connect(QtGui.qApp.aboutQt)

    def saveImageProcessing(self):
        s = "/* --- Meta Data - START ---\n"
        s+= "    type " + wrapper_type + "\n"
        for b in self.parametersWidget.inputBuffers:
            s+= "    input_buffer " + b.name + " " + b.lineEdit.text() + "\n"
        for b in self.parametersWidget.outputBuffers:
            s+= "    output_buffer " + b.name + " " + b.lineEdit.text() + "\n"

        for param in self.parametersWidget.params:
            s+= "    %s %s %s %s %s %s\n" % (param.typename, 
                                        param.name, 
                                        str(eval(param.lineEdit.text())),
                                        str(param.defaultVal),
                                        str(param.minVal),
                                        str(param.maxVal))
        s+= "------ Meta Data - END --- */\n"
        s+=str(self.textEditor.toPlainText())

        filename = QtGui.QFileDialog.getSaveFileName(None, "Save Image Processing", 
                                                     QtCore.QDir.currentPath(), "IP (*.ip)")
        if filename[0]:
            with open(str(filename[0]), "w") as text_file:
                text_file.write(s)
                self.printLog("Writing Image Processing program to " + str(filename[0]) )

    def newImageProcessing(self):
        self.textEditor.clear()
        self.parametersWidget.clear()

    def openImageProcessing(self, filename = ""):
        global wrapper_type
        if filename == "":
            filename = QtGui.QFileDialog.getOpenFileName(None, "Open Image Processing", 
                                                         QtCore.QDir.currentPath(), "IP (*.ip)")
            filename = str(filename[0])
        
        if filename:
            self.parametersWidget.clear()

            with open(filename, "r") as text_file:
                s1 = "/* --- Meta Data - START ---\n"
                s2 = "------ Meta Data - END --- */\n"
                f = text_file.read()
                
                meta = f[f.find(s1) + len(s1):f.find(s2)].split("\n")
                for p in meta:
                    print p
                    v = p.split()
                    if not len(v):
                        continue
                    if v[0] == "type":
                        wrapper_type = v[1]
                        self.ip = PyIp.Wrapper(wrapper_type)
                    elif v[0] == "input_buffer":
                        self.parametersWidget.addInputBuffer(v[1])
                        if len(v) > 2:
                            self.parametersWidget.inputBuffers[-1].lineEdit.setText(v[2])
                    elif v[0] == "output_buffer":
                        self.parametersWidget.addOutputBuffer(v[1])
                        if len(v) > 2:
                            self.parametersWidget.outputBuffers[-1].lineEdit.setText(v[2])
                    elif v[0] == "double":
                        self.parametersWidget.addParameter(v[1], eval(v[3]), eval(v[4]), eval(v[5]))
                    elif v[0] == "int":
                        pass

                self.textEditor.clear()
                self.textEditor.appendPlainText(f[f.find(s2) + len(s2):])
                
    def createMenus(self):
        self.fileMenu = self.menuBar().addMenu("&File")
        self.fileMenu.addAction(self.newAct)
        self.fileMenu.addAction(self.openAct)
        self.fileMenu.addAction(self.saveAct)
        self.fileMenu.addSeparator()
        self.fileMenu.addAction(self.quitAct)

        self.runMenu = self.menuBar().addMenu("&Run")
        self.runMenu.addAction(self.buildAct)
        self.runMenu.addAction(self.runAct)
        
        self.helpMenu = self.menuBar().addMenu("&Help")
        #self.helpMenu.addAction(self.aboutAct)
        self.helpMenu.addAction(self.aboutQtAct)

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
        self.printLog("Running Image Processing with " + wrapper_type + \
                          "... took "+str(round(1000*dt,2))+" ms")
        self.gl.setData(self.outdata)
        self.gl.update()
        
        filename = str(self.parametersWidget.outputBuffers[0].lineEdit.text())
        if filename != "" and not self.interactiveCheckBox.isChecked():
            out = Image.fromarray(self.outdata.transpose(1,0,2))
            self.printLog("Saving output buffer to " + filename)
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
        self.debug = data
        self.gl.setData(data)
        self.gl.update()

        from time import time
        t = time()
        self.printLog("Building OpenCL kernel")
        self.ip.Build(str(self.textEditor.toPlainText()))
        dt = time()-t
        self.printLog("Building Image Processing with " + wrapper_type + "... took "+str(dt)+" seconds")
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
    global wrapper_type, code
    ip_file = sys.argv[1] if len(sys.argv) > 1 else ""

    app = QtGui.QApplication(sys.argv)
    app.setStyle("plastique")

    mainwindow = MainWindow(ip_file)
    mainwindow.setStyleSheet(darkorange_stylesheet.data)
    mainwindow.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()


    
