#This is an extension of refl_gui.py as that is a auto-generated script form pyqt and shouldn't be edited
#so this file provides any extra GUI tweaks not easily doable in the designer
#for the time being this also includes non-GUI behaviour
import refl_window
import csv
from PyQt4 import QtCore, QtGui
from mantid.simpleapi import *
from isis_reflectometry.quick import *
from isis_reflectometry.combineMulti import *
from isis_reflgui.latest_isis_runs import *
from mantid.api import Workspace, WorkspaceGroup

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

canMantidPlot = True

try:
    from mantidplot import *
except ImportError:
    canMantidPlot = False

class ReflGui(refl_window.Ui_windowRefl):
    __instrumentRuns = None

    def __del__(self):
        if self.windowRefl.modFlag:
            self.save(true)

    def on_buttonAuto_clicked(self):
        self.autoFill()
    def on_comboCycle_activated(self, cycle):
        self.populateList(selected_cycle=cycle)
    def on_buttonTransfer_clicked(self):
        self.transfer()
    def on_checkTickAll_stateChanged(self,state):
        self.unTickAll(state)
    def on_textRB_editingFinished(self):
        self.populateList()
    def on_buttonClear_clicked(self):
        self.initTable()
    def on_buttonProcess_clicked(self):
        self.process()
    def on_comboInstrument_activated(self, instrument):
        config['default.instrument'] = str(instrument)
        print "Instrument is now: ", str(instrument)
        self.populateList()
    def on_actionOpen_Table_triggered(self):
        self.loadTable()
    def on_actionReload_from_Disk_triggered(self):
        self.reloadTable()
    def on_actionSave_triggered(self):
        save()
    def on_actionSave_As_triggered(self):
        self.saveAs()
    def on_actionSave_Workspaces_triggered(self):
        self.saveWorkspaces()
    def actionClose_Refl_Gui_triggered(self):
        self.showHelp()
    def on_actionMantid_Help_triggered(self):
        self.showHelp()
    def on_tableMain_modified(self):
        if not self.loading:
            self.windowRefl.modFlag = True

    #Further UI setup
    def setupUi(self, windowRefl):
        super(ReflGui,self).setupUi(windowRefl)
        self.loading = False
        self.initTable()
        self.populateList()
        self.windowRefl = windowRefl
        self.connectSlots()
        #self.windowRefl.modFlag = True
    def initTable(self):
        self.currentTable = None
        self.tableMain.resizeColumnsToContents()
        instrumentList = ['INTER', 'SURF', 'CRISP', 'POLREF']
        currentInstrument = config['default.instrument']
        if currentInstrument in instrumentList:
            self.comboInstrument.setCurrentIndex(instrumentList.index(config['default.instrument'].upper()))
        else:
            self.comboInstrument.setCurrentIndex(0)
            config['default.instrument'] = 'INTER'
        for column in range(self.tableMain.columnCount()):
            for row in range(self.tableMain.rowCount()):
                if (column == 17):
                    check = QtGui.QCheckBox()
                    check.setCheckState(False)
                    item = QtGui.QWidget()
                    layout = QtGui.QHBoxLayout(item)
                    layout.addWidget(check)
                    layout.setAlignment(QtCore.Qt.AlignCenter)
                    layout.setSpacing(0)
                    layout.setContentsMargins(0, 0, 0, 0)
                    item.setLayout(layout)
                    item.setContentsMargins(0, 0, 0, 0)
                    self.tableMain.setCellWidget(row, 17, item)
                else:
                    item = QtGui.QTableWidgetItem()
                    item.setText('')
                    self.tableMain.setItem(row, column, item)
    def connectSlots(self):
        self.buttonAuto.clicked.connect(self.on_buttonAuto_clicked)
        self.checkTickAll.stateChanged.connect(self.on_checkTickAll_stateChanged)
        self.comboCycle.activated.connect(self.on_comboCycle_activated)
        self.comboInstrument.activated.connect(self.on_comboInstrument_activated)
        self.textRB.editingFinished.connect(self.on_textRB_editingFinished)
        self.buttonClear.clicked.connect(self.on_buttonClear_clicked)
        self.buttonProcess.clicked.connect(self.on_buttonProcess_clicked)
        self.buttonTransfer.clicked.connect(self.on_buttonTransfer_clicked)
        self.actionOpen_Table.triggered.connect(self.on_actionOpen_Table_triggered)
        self.actionReload_from_Disk.triggered.connect(self.on_actionReload_from_Disk_triggered)
        self.actionSave.triggered.connect(self.on_actionSave_triggered)
        self.actionSave_As.triggered.connect(self.on_actionSave_As_triggered)
        self.actionSave_Workspaces.triggered.connect(self.on_actionSave_Workspaces_triggered)
        self.actionClose_Refl_Gui.triggered.connect(self.windowRefl.close)
        self.actionMantid_Help.triggered.connect(self.on_actionMantid_Help_triggered)
        self.tableMain.cellChanged.connect(self.on_tableMain_modified)
    def populateList(self, selected_cycle=None):
        # Clear existing
        self.listMain.clear()
        # Fill with ADS workspaces
        self.populateListADSWorkspaces()
        try:
            selectedInstrument = config['default.instrument'].strip().upper()
            if not self.__instrumentRuns:
                self.__instrumentRuns =  LatestISISRuns(instrument=selectedInstrument)
            elif not self.__instrumentRuns.getInstrument() == selectedInstrument:
                self.__instrumentRuns =  LatestISISRuns(selectedInstrument)
            self.populateListCycle(selected_cycle)
        except Exception as ex:
            self.comboCycle.setVisible(False)
            logger.notice("Could not list archive runs")
            logger.notice(str(ex))

    #Functionality which will hopefully be moved into a new file
    def populateListCycle(self, selected_cycle=None):
        runs = self.__instrumentRuns.getJournalRuns(cycle=selected_cycle)
        for run in runs:
            self.listMain.addItem(run)
        # Get possible cycles for this instrument.
        cycles = self.__instrumentRuns.getCycles()
        # Setup the list of possible cycles. And choose the latest as the default
        if not selected_cycle:
            cycle_count = 0
            self.comboCycle.clear()
            for cycle in cycles:
                self.comboCycle.addItem(cycle)
                if cycle == self.__instrumentRuns.getLatestCycle():
                    self.comboCycle.setCurrentIndex(cycle_count)
                cycle_count+=1
        # Ensure that the cycle widget is shown.
        self.comboCycle.setVisible(True)
    def populateListADSWorkspaces(self):
        names = mtd.getObjectNames()
        for ws in names:
            self.listMain.addItem(ws)
    def autoFill(self):
        col = 0
        # make sure all selected cells are in the same row
        sum = 0
        howMany = len(self.tableMain.selectedItems())
        for cell in self.tableMain.selectedItems():
            sum = sum + self.tableMain.row(cell)
        if (howMany):
            if (sum / howMany == self.tableMain.row(self.tableMain.selectedItems()[0])):
                for cell in self.tableMain.selectedItems():
                    row = self.tableMain.row(cell) + 1
                    txt = cell.text()
                    while (self.tableMain.item(row, 0).text() != ''):
                        item = QtGui.QTableWidgetItem()
                        item.setText(txt)
                        self.tableMain.setItem(row, self.tableMain.column(cell), item)
                        row = row + 1
    def transfer(self):
        col = 0
        row = 0
        while (self.tableMain.item(row, 0).text() != ''):
            row = row + 1
        for idx in self.listMain.selectedItems():
            contents = str(idx.text()).strip()
            first_contents = contents.split(':')[0]
            runnumber = None
            if mtd.doesExist(first_contents):
                runnumber = groupGet(mtd[first_contents], "samp", "run_number")
            else:
                temp = Load(Filename=first_contents, OutputWorkspace="_tempforrunnumber")
                runnumber = groupGet("_tempforrunnumber", "samp", "run_number")
                DeleteWorkspace(temp)
            item = QtGui.QTableWidgetItem()
            item.setText(runnumber)
            self.tableMain.setItem(row, col, item)
            item = QtGui.QTableWidgetItem()
            item.setText(self.textRuns.text())
            self.tableMain.setItem(row, col + 2, item)
            col = col + 5
            if col >= 11:
                col = 0
                row = row + 1
    def unTickAll(self,state):
        for row in range(self.tableMain.rowCount()):
            self.tableMain.cellWidget(row, 17).children()[1].setCheckState(state)
    def process(self):
#--------- If "Process" button pressed, convert raw files to IvsLam and IvsQ and combine if checkbox ticked -------------
        willProcess = True
        rows = self.tableMain.selectionModel().selectedRows()
        rowIndexes=[]
        for idx in rows:
            rowIndexes.append(idx.row())
        if not len(rowIndexes):
            reply = QtGui.QMessageBox.question(self.tableMain, 'Process all rows?',"This will process all rows in the table. Continue?", QtGui.QMessageBox.Yes, QtGui.QMessageBox.No)
            if reply == QtGui.QMessageBox.No:
                print "Cancelled!"
                willProcess = False
            else:
                rowIndexes = range(self.tableMain.rowCount())
        if willProcess:
            for row in rowIndexes:  # range(self.tableMain.rowCount()):
                runno = []
                wksp = []
                wkspBinned = []
                overlapLow = []
                overlapHigh = []
                g = ['g1', 'g2', 'g3']
                theta = [0, 0, 0]
                if (self.tableMain.item(row, 0).text() != ''):
                    for i in range(3):
                        r = str(self.tableMain.item(row, i * 5).text())
                        if (r != ''):
                            runno.append(r)
                        ovLow = str(self.tableMain.item(row, i * 5 + 3).text())
                        if (ovLow != ''):
                            overlapLow.append(float(ovLow))
                        ovHigh = str(self.tableMain.item(row, i * 5 + 4).text())
                        if (ovHigh != ''):
                            overlapHigh.append(float(ovHigh))
                    print len(runno), "runs: ", runno
                    # Determine resolution
                    # if (runno[0] != ''):
                    if (self.tableMain.item(row, 15).text() == ''):
                        dqq = calcRes(runno[0])
                        item = QtGui.QTableWidgetItem()
                        item.setText(str(dqq))
                        self.tableMain.setItem(row, 15, item)
                        print "Calculated resolution: ", dqq
                    else:
                        dqq = float(self.tableMain.item(row, 15).text())
                    # Populate runlist
                    for i in range(len(runno)):
                        [theta, qmin, qmax] = self.dorun(runno[i], row, i)
                        theta = round(theta, 3)
                        qmin = round(qmin, 3)
                        qmax = round(qmax, 3)
                        wksp.append(runno[i] + '_IvsQ')
                        if (self.tableMain.item(row, i * 5 + 1).text() == ''):
                            item = QtGui.QTableWidgetItem()
                            item.setText(str(theta))
                            self.tableMain.setItem(row, i * 5 + 1, item)
                        if (self.tableMain.item(row, i * 5 + 3).text() == ''):
                            item = QtGui.QTableWidgetItem()
                            item.setText(str(qmin))
                            self.tableMain.setItem(row, i * 5 + 3, item)
                            overlapLow.append(qmin)
                        if (self.tableMain.item(row, i * 5 + 4).text() == ''):
                            item = QtGui.QTableWidgetItem()
                            if i == len(runno) - 1:
                            # allow full high q-range for last angle
                                qmax = 4 * math.pi / ((4 * math.pi / qmax * math.sin(theta * math.pi / 180)) - 0.5) * math.sin(theta * math.pi / 180)
                            item.setText(str(qmax))
                            self.tableMain.setItem(row, i * 5 + 4, item)
                            overlapHigh.append(qmax)
                        if wksp[i].find(',') > 0 or wksp[i].find(':') > 0:
                            runlist = []
                            l1 = wksp[i].split(',')
                            for subs in l1:
                                l2 = subs.split(':')
                                for l3 in l2:
                                    runlist.append(l3)
                            wksp[i] = runlist[0] + '_IvsQ'
                        ws_name_binned = wksp[i] + '_binned'
                        ws = getWorkspace(wksp[i])
                        w1 = getWorkspace(wksp[0])
                        w2 = getWorkspace(wksp[len(wksp) - 1])
                        if len(overlapLow):
                            Qmin = overlapLow[0]
                        else:
                            Qmin = w1.readX(0)[0]
                        if len(overlapHigh):
                            Qmax = overlapHigh[len(overlapHigh) - 1]
                        else:
                            Qmax = max(w2.readX(0))
                        Rebin(InputWorkspace=wksp[i], Params=str(overlapLow[i]) + ',' + str(-dqq) + ',' + str(overlapHigh[i]), OutputWorkspace=ws_name_binned)
                        wkspBinned.append(ws_name_binned)
                        wsb = getWorkspace(ws_name_binned)
                        Imin = min(wsb.readY(0))
                        Imax = max(wsb.readY(0))
                        if canMantidPlot:
                            g[i] = plotSpectrum(ws_name_binned, 0, True)
                            titl = groupGet(ws_name_binned, 'samp', 'run_title')
                            if (i > 0):
                                mergePlots(g[0], g[i])
                            if (type(titl) == str):
                                g[0].activeLayer().setTitle(titl)
                            g[0].activeLayer().setAxisScale(Layer.Left, Imin * 0.1, Imax * 10, Layer.Log10)
                            g[0].activeLayer().setAxisScale(Layer.Bottom, Qmin * 0.9, Qmax * 1.1, Layer.Log10)
                            g[0].activeLayer().setAutoScale()
                    if (self.tableMain.cellWidget(row, 17).children()[1].checkState() > 0):
                        if (len(runno) == 1):
                            print "Nothing to combine!"
                        elif (len(runno) == 2):
                            outputwksp = runno[0] + '_' + runno[1][3:5]
                        else:
                            outputwksp = runno[0] + '_' + runno[2][3:5]
                        print runno
                        w1 = getWorkspace(wksp[0])
                        w2 = getWorkspace(wksp[len(wksp) - 1])
                        begoverlap = w2.readX(0)[0]
                        # Qmin = w1.readX(0)[0]
                        # Qmax = max(w2.readX(0))
                        # get Qmax
                        if (self.tableMain.item(row, i * 5 + 4).text() == ''):
                            overlapHigh = 0.3 * max(w1.readX(0))
                        print overlapLow, overlapHigh
                        wcomb = combineDataMulti(wkspBinned, outputwksp, overlapLow, overlapHigh, Qmin, Qmax, -dqq, 1)
                        if (self.tableMain.item(row, 16).text() != ''):
                            Scale(InputWorkspace=outputwksp, OutputWorkspace=outputwksp, Factor=1 / float(self.tableMain.item(row, 16).text()))
                        Qmin = getWorkspace(outputwksp).readX(0)[0]
                        Qmax = max(getWorkspace(outputwksp).readX(0))
                        if canMantidPlot:
                            gcomb = plotSpectrum(outputwksp, 0, True)
                            titl = groupGet(outputwksp, 'samp', 'run_title')
                            gcomb.activeLayer().setTitle(titl)
                            gcomb.activeLayer().setAxisScale(Layer.Left, 1e-8, 100.0, Layer.Log10)
                            gcomb.activeLayer().setAxisScale(Layer.Bottom, Qmin * 0.9, Qmax * 1.1, Layer.Log10)
    def dorun(self, runno, row, which):
        g = ['g1', 'g2', 'g3']
        transrun = str(self.tableMain.item(row, which * 5 + 2).text())
        angle = str(self.tableMain.item(row, which * 5 + 1).text())
        names = mtd.getObjectNames()
        [wlam, wq, th] = quick(runno, trans=transrun, theta=angle)
        if ':' in runno:
            runno = runno.split(':')[0]
        if ',' in runno:
            runno = runno.split(',')[0]

        ws_name = str(runno) + '_IvsQ'
        inst = groupGet(ws_name, 'inst')
        lmin = inst.getNumberParameter('LambdaMin')[0] + 1
        lmax = inst.getNumberParameter('LambdaMax')[0] - 2
        qmin = 4 * math.pi / lmax * math.sin(th * math.pi / 180)
        qmax = 4 * math.pi / lmin * math.sin(th * math.pi / 180)
        return th, qmin, qmax
    def saveTable(self, filename):
        try:
            writer = csv.writer(open(filename, "wb"))
            for row in range(self.tableMain.rowCount()):
                rowtext = []
                for column in range(self.tableMain.columnCount() - 1):
                    rowtext.append(self.tableMain.item(row, column).text())
                if (len(rowtext) > 0):
                    writer.writerow(rowtext)
            self.currentTable = filename
            print "Saved file to " + filename
            self.windowRefl.modFlag = False
        except:
            return False
        self.windowRefl.modFlag = False
        return True
    def save(self, failsave = False):
        filename = ''
        if failsave:
            #this is an emergency autosave as the program is failing
            print "The ISIS Reflectonomy GUI has encountered an error, it will now attempt to save a copy of your work."
            msgBox = QtGui.QMessageBox()
            msgBox.setText("The ISIS Reflectonomy GUI has encountered an error, it will now attempt to save a copy of your work.\nPlease check the log for details.")
            msgBox.setStandardButtons(QtGui.QMessageBox.Ok)
            msgBox.setIcon(QtGui.QMessageBox.Critical)
            msgBox.setDefaultButton(QtGui.QMessageBox.Ok)
            msgBox.setEscapeButton(QtGui.QMessageBox.Ok)
            msgBox.exec_()
            import datetime
            failtime = datetime.datetime.today().strftime('%Y-%m-%d_%H-%M-%S')
            if self.currentTable:
                filename = self.currentTable.rsplit('.',1)[0] + "_recovered_" + failtime + ".tbl"
            else:
                mantidDefault = config['defaultsave.directory']
                if os.path.exists(mantidDefault):
                    filename = os.path.join(mantidDefault,"mantid_reflectometry_recovered_" + failtime + ".tbl")
                else:
                    import tempfile
                    tempDir = tempfile.gettempdir()
                    filename = os.path.join(tempDir,"mantid_reflectometry_recovered_" + failtime + ".tbl")
        else:
            #this is a save-on-quit or file->save
            if self.currentTable:
                filename = self.currentTable
            else:
                saveDialog = QtGui.QFileDialog(self.layoutMainRow.parent(), "Save Table")
                saveDialog.setFileMode(QtGui.QFileDialog.AnyFile)
                saveDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
                saveDialog.setDefaultSuffix("tbl")
                saveDialog.setAcceptMode(QtGui.QFileDialog.AcceptSave)
                if saveDialog.exec_():
                    filename = saveDialog.selectedFiles()[0]
                else:
                    return False
        return self.saveTable(filename)
    def saveAs(self):
        saveDialog = QtGui.QFileDialog(self.layoutMainRow.parent(), "Save Table")
        saveDialog.setFileMode(QtGui.QFileDialog.AnyFile)
        saveDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
        saveDialog.setDefaultSuffix("tbl")
        saveDialog.setAcceptMode(QtGui.QFileDialog.AcceptSave)
        if saveDialog.exec_():
            filename = saveDialog.selectedFiles()[0]
            self.saveTable(filename)
    def loadTable(self):
        self.loading = True
        loadDialog = QtGui.QFileDialog(self.layoutMainRow.parent(), "Open Table")
        loadDialog.setFileMode(QtGui.QFileDialog.ExistingFile)
        loadDialog.setNameFilter("Table Files (*.tbl);;All files (*.*)")
        if loadDialog.exec_():
            try:
                filename = loadDialog.selectedFiles()[0]
                self.currentTable = filename
                reader = csv.reader(open(filename, "rb"))
                row = 0
                for line in reader:
                    if (row < 100):
                        for column in range(self.tableMain.columnCount() - 1):
                            item = QtGui.QTableWidgetItem()
                            item.setText(line[column])
                            self.tableMain.setItem(row, column, item)
                        row = row + 1
            except:
                print 'Could not load file: ' + filename + '. File not found or unable to read from file.'
        self.loading = False
        self.windowRefl.modFlag = False
    def reloadTable(self):
        self.loading = True
        filename = self.currentTable
        if filename:
            try:
                reader = csv.reader(open(filename, "rb"))
                row = 0
                for line in reader:
                    if (row < 100):
                        for column in range(self.tableMain.columnCount() - 1):
                            item = QtGui.QTableWidgetItem()
                            item.setText(line[column])
                            self.tableMain.setItem(row, column, item)
                        row = row + 1
            except:
                print 'Could not load file: ' + filename + '. File not found or unable to read from file.'
        else:
            print 'No file in table to reload.'
        self.loading = False
        self.windowRefl.modFlag = False
    def saveWorkspaces(self):
        #Dialog = QtGui.QDialog()
        #u = Ui_SaveWindow()
        #u.setupUi(Dialog)
        #Dialog.exec_()
        #print "Disabled until decision about old dialog has been reached"
        print "Disabled - Run desired save algorithm from main MantidPlot window instead"
    def showHelp(self):
        import webbrowser
        webbrowser.open('http://www.mantidproject.org/ISIS_Reflectometry_GUI')

def calcRes(run):
    runno = '_' + str(run) + 'temp'
    if type(run) == type(int()):
        Load(Filename=run, OutputWorkspace=runno)
    else:
        Load(Filename=run.replace("raw", "nxs", 1), OutputWorkspace=runno)
    # Get slits and detector angle theta from NeXuS
    theta = groupGet(runno, 'samp', 'THETA')
    inst = groupGet(runno, 'inst')
    s1z = inst.getComponentByName('slit1').getPos().getZ() * 1000.0  # distance in mm
    s2z = inst.getComponentByName('slit2').getPos().getZ() * 1000.0  # distance in mm
    s1vg = inst.getComponentByName('slit1')
    s1vg = s1vg.getNumberParameter('vertical gap')[0]
    s2vg = inst.getComponentByName('slit2')
    s2vg = s2vg.getNumberParameter('vertical gap')[0]
    if type(theta) != float:
        th = theta[len(theta) - 1]
    else:
        th = theta
    print "s1vg=", s1vg, "s2vg=", s2vg, "theta=", theta
    #1500.0 is the S1-S2 distance in mm for SURF!!!
    resolution = math.atan((s1vg + s2vg) / (2 * (s2z - s1z))) * 180 / math.pi / th
    print "dq/q=", resolution
    DeleteWorkspace(runno)
    return resolution
def groupGet(wksp, whattoget, field=''):
    '''
    returns information about instrument or sample details for a given workspace wksp,
    also if the workspace is a group (info from first group element)
    '''
    if (whattoget == 'inst'):
        if isinstance(wksp, str):
            if isinstance(mtd[wksp], WorkspaceGroup):
                return mtd[wksp + '_1'].getInstrument()
            else:
                return mtd[wksp].getInstrument()
        elif isinstance(wksp, Workspace):
            if isinstance(wksp, WorkspaceGroup):
                return mtd[wksp + '_1'].getInstrument()
            else:
                return wksp.getInstrument()
        else:
            return 0
    elif (whattoget == 'samp' and field != ''):
        if isinstance(wksp, str):
            if isinstance(mtd[wksp], WorkspaceGroup):
                try:
                    log = mtd[wksp + '_1'].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[len(log) - 1]
                except RuntimeError:
                    res = 0
                    print "Block " + field + " not found."
            else:
                try:
                    log = mtd[wksp].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[len(log) - 1]
                except RuntimeError:
                    res = 0
                    print "Block " + field + " not found."
        elif isinstance(wksp, Workspace):
            if isinstance(wksp, WorkspaceGroup):
                try:
                    log = mtd[wksp + '_1'].getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[len(log) - 1]
                except RuntimeError:
                    res = 0
                    print "Block " + field + " not found."
            else:
                try:
                    log = wksp.getRun().getLogData(field).value
                    if (type(log) is int or type(log) is str):
                        res = log
                    else:
                        res = log[len(log) - 1]
                except RuntimeError:
                    res = 0
                    print "Block " + field + " not found."
        else:
            res = 0
        return res
    elif (whattoget == 'wksp'):
        if isinstance(wksp, str):
            if isinstance(mtd[wksp], WorkspaceGroup):
                return mtd[wksp + '_1'].getNumberHistograms()
            else:
                return mtd[wksp].getNumberHistograms()
        elif isinstance(wksp, Workspace):
            if isinstance(wksp, WorkspaceGroup):
                return mtd[wksp + '_1'].getNumberHistograms()
            else:
                return wksp.getNumberHistograms()
        else:
            return 0
def getWorkspace(wksp):
    if isinstance(wksp, Workspace):
        return wksp
    elif isinstance(wksp, str):
        if isinstance(mtd[wksp], WorkspaceGroup):
            wout = mtd[wksp + '_1']
        else:
            wout = mtd[wksp]
        return wout
    else:
        print "Unable to get workspace: " + wksp
        return 0