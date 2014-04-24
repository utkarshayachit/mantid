# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'D:\mantid\windows\Code\Mantid\scripts\Interface\ui\reflectometer/refl_save_window.ui'
#
# Created: Tue Apr 15 14:09:46 2014
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class ReflSave(QtGui.QDialog, refl_save_window.Ui_SaveDialog):

    def __init__(self):
        """
        Initialise the interface
        """
        super(QtGui.QDialog, self).__init__()
        self.setupUi(self)
        self.__instrument = config['default.instrument'].strip().upper()
        
        try:
            usersettings = Settings() # This will throw a missing config exception if no config file is available.
            self.__mountpoint = usersettings.get_named_setting("DataMountPoint")
        except KeyError:
            print "DataMountPoint is missing from the config.xml file."
            raise
        self.SavePath=""
        
        self.populateList()
        self.buttonsMain.clicked.connect(self.check_save_clicked)
        self.buttonBrowse.clicked.connect()
        self.buttonTransfer.clicked.connect()
        self.listWorkspaces.itemActivated.connect(self.workspaceSelected)
        
    def check_save_clicked(self, button):
        if self.buttonsColumns.button(QtGui.QDialogButtonBox.Save) == button:
            self.save()
    
    def save(self):
        names = mtd.getObjectNames()
        dataToSave=[]
        prefix = str(self.lineEdit2.text())
        if not (self.lineEdit.text() and os.path.exists(self.lineEdit.text())):
            logger.notice("Directory specified doesn't exist or was invalid for your operating system")
            QtGui.QMessageBox.critical(self.lineEdit, 'Could not save',"Directory specified doesn't exist or was invalid for your operating system")
            return
        for idx in self.listWidget.selectedItems():
            runlist=parseRunList(str(self.spectraEdit.text()))
            fname=os.path.join(self.lineEdit.text(),prefix + idx.text())
            if (self.comboBox.currentIndex() == 0):
                print "Custom Ascii format"
                if (self.radio1.isChecked()):
                    sep=','
                elif (self.radio2.isChecked()):
                    sep=' '
                elif (self.radio3.isChecked()):
                    sep='\t'
                else:
                    sep=' '
                saveCustom(idx,fname,sep,self.listWidget2.selectedItems(),self.titleCheckBox.isChecked(),self.xErrorCheckBox.isChecked())
            elif (self.comboBox.currentIndex() == 1):
                print "Not yet implemented!"
            elif (self.comboBox.currentIndex() == 2):
                print "ANSTO format"
                saveANSTO(idx,fname)
            elif (self.comboBox.currentIndex() == 3):
                print "ILL MFT format"
                saveMFT(idx,fname,self.listWidget2.selectedItems())
        # for idx in self.listWidget.selectedItems():
            # fname=str(path+prefix+idx.text()+'.dat')
            # print "FILENAME: ", fname
            # wksp=str(idx.text())
            # SaveAscii(InputWorkspace=wksp,Filename=fname)
        self.SavePath=self.lineEdit.text()

    def workspaceSelected(self):
        self.listWidget2.clear()
        #self.listWidget.setCurrentRow(0)
        print str(self.listWidget.currentItem().text())
        logs = mtd[str(self.listWidget.currentItem().text())].getRun().getLogData()
        for i in range(0,len(logs)):
            self.listWidget2.addItem(logs[i].name)
"""
    def populateList(self):
        self.listWorkspaces.clear()
        self.listParameters.clear()
        names = mtd.getObjectNames()
        if len(names):
            RB_Number=groupGet(names[0],'samp','rb_proposal')
            for ws in names:
                self.listWidget.addItem(ws)
                
            self.listWidget.setCurrentItem(self.listWidget.item(0))
            # try to get correct user directory
            currentInstrument=config['default.instrument']
            if (self.SavePath!=''):
                self.lineEdit.setText(self.SavePath)
            else:
                base_path = os.path.join(self.__mountpoint, 'NDX'+  self.__instrument, 'Instrument','logs','journal')
                print base_path
                main_journal_path = os.path.join(base_path, 'journal_main.xml')
                tree1=xml.parse(main_journal_path)
                root1=tree1.getroot()
                currentJournal=root1[len(root1)-1].attrib.get('name')
                cycle_journal_path = os.path.join(base_path, currentJournal)
                tree=xml.parse(cycle_journal_path)
                root=tree.getroot()
                
                i=0
                try:
                    while root[i][4].text!=str(RB_Number):
                        i+=1
                    if root[i][1].text.find(',')>0:
                        user=root[i][1].text[0:root[i][1].text.find(',')]
                    else:
                        user=root[i][1].text[0:root[i][1].text.find(' ')]
                    SavePath = os.path.join('U:', user)
                    self.lineEdit.setText(SavePath)
                except LookupError:
                    print "Not found!"

#--------- If "Save" button pressed, selcted workspaces are saved -------------
    def buttonClickHandler1(self):
        names = mtd.getObjectNames()
        dataToSave=[]
        prefix = str(self.lineEdit2.text())
        if not (self.lineEdit.text() and os.path.exists(self.lineEdit.text())):
            logger.notice("Directory specified doesn't exist or was invalid for your operating system")
            QtGui.QMessageBox.critical(self.lineEdit, 'Could not save',"Directory specified doesn't exist or was invalid for your operating system")
            return
        for idx in self.listWidget.selectedItems():
            runlist=parseRunList(str(self.spectraEdit.text()))
            fname=os.path.join(self.lineEdit.text(),prefix + idx.text())
            if (self.comboBox.currentIndex() == 0):
                print "Custom Ascii format"
                if (self.radio1.isChecked()):
                    sep=','
                elif (self.radio2.isChecked()):
                    sep=' '
                elif (self.radio3.isChecked()):
                    sep='\t'
                else:
                    sep=' '
                saveCustom(idx,fname,sep,self.listWidget2.selectedItems(),self.titleCheckBox.isChecked(),self.xErrorCheckBox.isChecked())
            elif (self.comboBox.currentIndex() == 1):
                print "Not yet implemented!"
            elif (self.comboBox.currentIndex() == 2):
                print "ANSTO format"
                saveANSTO(idx,fname)
            elif (self.comboBox.currentIndex() == 3):
                print "ILL MFT format"
                saveMFT(idx,fname,self.listWidget2.selectedItems())
        # for idx in self.listWidget.selectedItems():
            # fname=str(path+prefix+idx.text()+'.dat')
            # print "FILENAME: ", fname
            # wksp=str(idx.text())
            # SaveAscii(InputWorkspace=wksp,Filename=fname)
            
        self.SavePath=self.lineEdit.text()

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
        if isinstance(mtd[wksp], WorkspaceGroup):
            return mtd[wksp + '_1'].getInstrument()
        else:
            return mtd[wksp].getInstrument()
    elif (whattoget == 'samp' and field != ''):
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
        return res
    elif (whattoget == 'wksp'):
        if isinstance(mtd[wksp], WorkspaceGroup):
            return mtd[wksp + '_1'].getNumberHistograms()
        else:
            return mtd[wksp].getNumberHistograms()

def getWorkspace(wksp):

    if isinstance(mtd[wksp], WorkspaceGroup):
        wout = mtd[wksp + '_1']
    else:
        wout = mtd[wksp]
    return wout
"""