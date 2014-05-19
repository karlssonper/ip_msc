from PySide import QtGui, QtCore



class ParametersWidget(QtGui.QWidget):
    def __init__(self, parent, paramCallbackFunc = None):
        super(ParametersWidget, self).__init__(parent)
        self.paramCallbackFunc = paramCallbackFunc
        self.params = []
        self.inputBuffers = []
        self.outputBuffers = []

        inputBuffersBox = QtGui.QGroupBox("Input buffers")
        outputBuffersBox = QtGui.QGroupBox("Output buffers")
        parametersBox = QtGui.QGroupBox("Parameters")

        self.inputBuffersGrid = QtGui.QGridLayout()
        self.outputBuffersGrid = QtGui.QGridLayout()
        self.parametersGrid = QtGui.QGridLayout()

        inputBuffersBox.setLayout(self.inputBuffersGrid)
        outputBuffersBox.setLayout(self.outputBuffersGrid)
        parametersBox.setLayout(self.parametersGrid)
        
        vlay = QtGui.QVBoxLayout()
        vlay.addWidget(inputBuffersBox)
        vlay.addWidget(outputBuffersBox)
        vlay.addWidget(parametersBox)
        self.setLayout(vlay)
    
    def addInputBuffer(self, name):
        self.inputBuffers.append(
            Buffer(name, self.inputBuffersGrid, len(self.inputBuffers), False))
           
    def addOutputBuffer(self, name):
         self.outputBuffers.append(
             Buffer(name, self.outputBuffersGrid, len(self.outputBuffers), True))
          
    def addParameter(self, name, defaultVal, minVal, maxVal, typename = "double"):
        self.params.append(
            Parameter(self.parametersGrid, len(self.params),
                           name, defaultVal, minVal, maxVal, typename, self.paramCallbackFunc))
        

class Buffer(object):
    def __init__(self, name, grid, row, save = True):
        label = QtGui.QLabel(name)
        self.lineEdit = QtGui.QLineEdit()
        self.btn = QtGui.QPushButton("..")
        grid.addWidget(label, row, 0)
        grid.addWidget(self.lineEdit, row, 1)
        grid.addWidget(self.btn, row, 2)
        self.btn.clicked.connect(self.updateLineEdit)
        self.save = save

    def updateLineEdit(self):
        func = QtGui.QFileDialog.getOpenFileName if not self.save else \
               QtGui.QFileDialog.getSaveFileName
        filename = func(None, "Select Buffer", QtCore.QDir.currentPath(), "Png (*.png)")
        if filename[0]:
            self.lineEdit.setText(filename[0])

class Parameter(object):
    def __init__(self, grid, row, name, defaultVal, minVal, maxVal, typename, callbackFunc):
        self.callbackFunc = callbackFunc
        label = QtGui.QLabel(name)
        self.name = name
        self.lineEdit = QtGui.QLineEdit()
        self.slider = QtGui.QSlider(QtCore.Qt.Horizontal)
        grid.addWidget(label, row, 0)
        grid.addWidget(self.lineEdit, row, 1)
        grid.addWidget(self.slider, row, 2)

        self.lineEdit.setFixedWidth(40)
        self.slider.setRange(0,100)
        self.minVal = minVal
        self.maxVal = maxVal
        self.defaultVal = defaultVal
        self.typename = typename

        self.lineEdit.textChanged.connect(self.onTextChange)
        self.slider.valueChanged.connect(self.onSliderChange)

        self.updateSlider = True
        self.updateLineEdit = True
        
        txt = str(int(defaultVal)) if typename == "int" else str(defaultVal)
        self.lineEdit.setText(txt)

      

    def onTextChange(self):
        if not self.updateLineEdit:
            return

        try:
            val = eval(self.lineEdit.text())
            if self.typename == "int":
                val = int(val)
                self.lineEdit.setText(str(val))
        except SyntaxError:
                val = self.defaultVal
        
        self.updateSlider = False 
        if val < self.minVal:
            self.slider.setSliderPosition(0)
        elif val > self.maxVal:
            self.slider.setSliderPosition(100)
        else:
            self.slider.setSliderPosition( 100*(val-self.minVal) / float(self.maxVal-self.minVal))
        self.updateSlider = True

        if self.callbackFunc:
            self.callbackFunc()

    def onSliderChange(self, val):
        if not self.updateSlider:
            return 
        
        val = 0.01 * self.slider.value() * (self.maxVal-self.minVal) + self.minVal
        txt = str(int(val))if self.typename == "int" else str(val)
    
        self.updateLineEdit = False
        self.lineEdit.setText(txt)
        self.updateLineEdit = True

        if self.callbackFunc:
            self.callbackFunc()
    
