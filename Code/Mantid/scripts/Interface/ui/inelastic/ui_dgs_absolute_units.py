# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/inelastic/dgs_absolute_units.ui'
#
# Created: Mon Jun 18 16:12:24 2012
#      by: PyQt4 UI code generator 4.7.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_AbsUnitsFrame(object):
    def setupUi(self, AbsUnitsFrame):
        AbsUnitsFrame.setObjectName("AbsUnitsFrame")
        AbsUnitsFrame.resize(1011, 514)
        AbsUnitsFrame.setFrameShape(QtGui.QFrame.StyledPanel)
        AbsUnitsFrame.setFrameShadow(QtGui.QFrame.Raised)
        self.verticalLayout_5 = QtGui.QVBoxLayout(AbsUnitsFrame)
        self.verticalLayout_5.setObjectName("verticalLayout_5")
        self.absunits_gb = QtGui.QGroupBox(AbsUnitsFrame)
        self.absunits_gb.setCheckable(True)
        self.absunits_gb.setChecked(False)
        self.absunits_gb.setObjectName("absunits_gb")
        self.verticalLayout_4 = QtGui.QVBoxLayout(self.absunits_gb)
        self.verticalLayout_4.setObjectName("verticalLayout_4")
        self.run_files_gb = QtGui.QGroupBox(self.absunits_gb)
        self.run_files_gb.setObjectName("run_files_gb")
        self.verticalLayout = QtGui.QVBoxLayout(self.run_files_gb)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.absunits_van_label = QtGui.QLabel(self.run_files_gb)
        self.absunits_van_label.setMinimumSize(QtCore.QSize(225, 0))
        self.absunits_van_label.setObjectName("absunits_van_label")
        self.horizontalLayout.addWidget(self.absunits_van_label)
        self.absunits_van_edit = QtGui.QLineEdit(self.run_files_gb)
        self.absunits_van_edit.setObjectName("absunits_van_edit")
        self.horizontalLayout.addWidget(self.absunits_van_edit)
        self.absunits_van_browse = QtGui.QPushButton(self.run_files_gb)
        self.absunits_van_browse.setObjectName("absunits_van_browse")
        self.horizontalLayout.addWidget(self.absunits_van_browse)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtGui.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.grouping_file_label = QtGui.QLabel(self.run_files_gb)
        self.grouping_file_label.setMinimumSize(QtCore.QSize(225, 0))
        self.grouping_file_label.setObjectName("grouping_file_label")
        self.horizontalLayout_2.addWidget(self.grouping_file_label)
        self.grouping_file_edit = QtGui.QLineEdit(self.run_files_gb)
        self.grouping_file_edit.setObjectName("grouping_file_edit")
        self.horizontalLayout_2.addWidget(self.grouping_file_edit)
        self.grouping_file_browse = QtGui.QPushButton(self.run_files_gb)
        self.grouping_file_browse.setObjectName("grouping_file_browse")
        self.horizontalLayout_2.addWidget(self.grouping_file_browse)
        spacerItem1 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)
        self.verticalLayout.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3 = QtGui.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.absunits_detvan_label = QtGui.QLabel(self.run_files_gb)
        self.absunits_detvan_label.setMinimumSize(QtCore.QSize(225, 0))
        self.absunits_detvan_label.setObjectName("absunits_detvan_label")
        self.horizontalLayout_3.addWidget(self.absunits_detvan_label)
        self.absunits_detvan_edit = QtGui.QLineEdit(self.run_files_gb)
        self.absunits_detvan_edit.setObjectName("absunits_detvan_edit")
        self.horizontalLayout_3.addWidget(self.absunits_detvan_edit)
        self.absunits_detvan_browse = QtGui.QPushButton(self.run_files_gb)
        self.absunits_detvan_browse.setObjectName("absunits_detvan_browse")
        self.horizontalLayout_3.addWidget(self.absunits_detvan_browse)
        spacerItem2 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem2)
        self.verticalLayout.addLayout(self.horizontalLayout_3)
        self.verticalLayout_4.addWidget(self.run_files_gb)
        self.integration_gb = QtGui.QGroupBox(self.absunits_gb)
        self.integration_gb.setObjectName("integration_gb")
        self.verticalLayout_2 = QtGui.QVBoxLayout(self.integration_gb)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.horizontalLayout_4 = QtGui.QHBoxLayout()
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.ei_label = QtGui.QLabel(self.integration_gb)
        self.ei_label.setMinimumSize(QtCore.QSize(225, 0))
        self.ei_label.setObjectName("ei_label")
        self.horizontalLayout_4.addWidget(self.ei_label)
        self.ei_edit = QtGui.QLineEdit(self.integration_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.ei_edit.sizePolicy().hasHeightForWidth())
        self.ei_edit.setSizePolicy(sizePolicy)
        self.ei_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.ei_edit.setObjectName("ei_edit")
        self.horizontalLayout_4.addWidget(self.ei_edit)
        spacerItem3 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_4.addItem(spacerItem3)
        self.verticalLayout_2.addLayout(self.horizontalLayout_4)
        self.horizontalLayout_5 = QtGui.QHBoxLayout()
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.emin_label = QtGui.QLabel(self.integration_gb)
        self.emin_label.setMinimumSize(QtCore.QSize(225, 0))
        self.emin_label.setObjectName("emin_label")
        self.horizontalLayout_5.addWidget(self.emin_label)
        self.emin_edit = QtGui.QLineEdit(self.integration_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.emin_edit.sizePolicy().hasHeightForWidth())
        self.emin_edit.setSizePolicy(sizePolicy)
        self.emin_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.emin_edit.setObjectName("emin_edit")
        self.horizontalLayout_5.addWidget(self.emin_edit)
        spacerItem4 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_5.addItem(spacerItem4)
        self.emax_label = QtGui.QLabel(self.integration_gb)
        self.emax_label.setObjectName("emax_label")
        self.horizontalLayout_5.addWidget(self.emax_label)
        self.emax_edit = QtGui.QLineEdit(self.integration_gb)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.emax_edit.sizePolicy().hasHeightForWidth())
        self.emax_edit.setSizePolicy(sizePolicy)
        self.emax_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.emax_edit.setObjectName("emax_edit")
        self.horizontalLayout_5.addWidget(self.emax_edit)
        spacerItem5 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_5.addItem(spacerItem5)
        self.verticalLayout_2.addLayout(self.horizontalLayout_5)
        self.verticalLayout_4.addWidget(self.integration_gb)
        self.masses_gb = QtGui.QGroupBox(self.absunits_gb)
        self.masses_gb.setObjectName("masses_gb")
        self.verticalLayout_3 = QtGui.QVBoxLayout(self.masses_gb)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.horizontalLayout_6 = QtGui.QHBoxLayout()
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.van_mass_label = QtGui.QLabel(self.masses_gb)
        self.van_mass_label.setMinimumSize(QtCore.QSize(225, 0))
        self.van_mass_label.setObjectName("van_mass_label")
        self.horizontalLayout_6.addWidget(self.van_mass_label)
        self.van_mass_edit = QtGui.QLineEdit(self.masses_gb)
        self.van_mass_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.van_mass_edit.setObjectName("van_mass_edit")
        self.horizontalLayout_6.addWidget(self.van_mass_edit)
        spacerItem6 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_6.addItem(spacerItem6)
        self.verticalLayout_3.addLayout(self.horizontalLayout_6)
        self.horizontalLayout_7 = QtGui.QHBoxLayout()
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        self.sample_mass_label = QtGui.QLabel(self.masses_gb)
        self.sample_mass_label.setMinimumSize(QtCore.QSize(225, 0))
        self.sample_mass_label.setObjectName("sample_mass_label")
        self.horizontalLayout_7.addWidget(self.sample_mass_label)
        self.sample_mass_edit = QtGui.QLineEdit(self.masses_gb)
        self.sample_mass_edit.setMinimumSize(QtCore.QSize(0, 0))
        self.sample_mass_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.sample_mass_edit.setObjectName("sample_mass_edit")
        self.horizontalLayout_7.addWidget(self.sample_mass_edit)
        spacerItem7 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_7.addItem(spacerItem7)
        self.verticalLayout_3.addLayout(self.horizontalLayout_7)
        self.horizontalLayout_8 = QtGui.QHBoxLayout()
        self.horizontalLayout_8.setObjectName("horizontalLayout_8")
        self.sample_rmm_label = QtGui.QLabel(self.masses_gb)
        self.sample_rmm_label.setMinimumSize(QtCore.QSize(225, 0))
        self.sample_rmm_label.setObjectName("sample_rmm_label")
        self.horizontalLayout_8.addWidget(self.sample_rmm_label)
        self.sample_rmm_edit = QtGui.QLineEdit(self.masses_gb)
        self.sample_rmm_edit.setMaximumSize(QtCore.QSize(100, 16777215))
        self.sample_rmm_edit.setObjectName("sample_rmm_edit")
        self.horizontalLayout_8.addWidget(self.sample_rmm_edit)
        spacerItem8 = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout_8.addItem(spacerItem8)
        self.verticalLayout_3.addLayout(self.horizontalLayout_8)
        self.verticalLayout_4.addWidget(self.masses_gb)
        spacerItem9 = QtGui.QSpacerItem(20, 171, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.verticalLayout_4.addItem(spacerItem9)
        self.verticalLayout_5.addWidget(self.absunits_gb)

        self.retranslateUi(AbsUnitsFrame)
        QtCore.QMetaObject.connectSlotsByName(AbsUnitsFrame)

    def retranslateUi(self, AbsUnitsFrame):
        AbsUnitsFrame.setWindowTitle(QtGui.QApplication.translate("AbsUnitsFrame", "Frame", None, QtGui.QApplication.UnicodeUTF8))
        self.absunits_gb.setTitle(QtGui.QApplication.translate("AbsUnitsFrame", "Perform Absolute Normalsation", None, QtGui.QApplication.UnicodeUTF8))
        self.run_files_gb.setTitle(QtGui.QApplication.translate("AbsUnitsFrame", "Run Files", None, QtGui.QApplication.UnicodeUTF8))
        self.absunits_van_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "AbsUnits Vanadium", None, QtGui.QApplication.UnicodeUTF8))
        self.absunits_van_browse.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.grouping_file_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Grouping File", None, QtGui.QApplication.UnicodeUTF8))
        self.grouping_file_browse.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.absunits_detvan_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Detector Vanadium (Abs Units)", None, QtGui.QApplication.UnicodeUTF8))
        self.absunits_detvan_browse.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Browse", None, QtGui.QApplication.UnicodeUTF8))
        self.integration_gb.setTitle(QtGui.QApplication.translate("AbsUnitsFrame", "Integration (meV)", None, QtGui.QApplication.UnicodeUTF8))
        self.ei_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Incident Energy", None, QtGui.QApplication.UnicodeUTF8))
        self.emin_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Energy range                  E Min", None, QtGui.QApplication.UnicodeUTF8))
        self.emax_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "E Max", None, QtGui.QApplication.UnicodeUTF8))
        self.masses_gb.setTitle(QtGui.QApplication.translate("AbsUnitsFrame", "Masses for Absolute Units", None, QtGui.QApplication.UnicodeUTF8))
        self.van_mass_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Vanadium mass", None, QtGui.QApplication.UnicodeUTF8))
        self.sample_mass_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Sample Mass", None, QtGui.QApplication.UnicodeUTF8))
        self.sample_rmm_label.setText(QtGui.QApplication.translate("AbsUnitsFrame", "Sample RMM", None, QtGui.QApplication.UnicodeUTF8))
