TEMPLATE = app
RC_FILE = Aluminizer2.rc
PRECOMPILED_HEADER = common.h
INCLUDEPATH += ../thirdparty/TNT ../thirdparty/JAMA
win32 { INCLUDEPATH += ../thirdparty/fftw ../thirdparty/ATL}

include(local.pro)

CONFIG += console precompile_header
CONFIG += debug

AL {
message("target = Aluminizer")
DEFINES += CONFIG_AL
TARGET = aluminizer
}

HG {
message("target = Mercurizer")
DEFINES += CONFIG_HG
TARGET = Mercurizer
}

CONFIG(debug, debug|release) {
win32 { LIBS += qwtd5.lib }
} else {
win32 { LIBS += qwt5.lib }
}

QT += network svg

DEFINES += CONFIG_PC HAS_HFS

LOC_NIST_LAPTOP {
message("location = NIST LAPTOP")
DEFINES += _NO_NIDAQ _NO_MCC_DIO _NO_MCC_AO
DEFINES += CONNECT_TO_SIM
}

LOC_TR_HOME {
message("location = TR Home")
DEFINES += _NO_NIDAQ _NO_MCC_DIO _NO_MCC_AO
DEFINES += CONNECT_TO_SIM
}

LOC_TR_OFFICE {
message("location = TR Office")
DEFINES += _NO_NIDAQ _NO_MCC_DIO _NO_MCC_AO CONNECT_TO_SIM
}

LOC_LAB_2034 {
message("location = LAB 2034")
DEFINES +=  _NO_MCC_DIO _HAS_OVENS
LIBS += ../LIB/cbw32.lib
}

LOC_LAB_2106 {
message("location = LAB 2106")
DEFINES +=  _NO_NIDAQ _NO_MCC_DIO _NO_MCC_AO
}

LOC_LAB_2041 {
message("location = LAB 2041")
DEFINES +=  _NO_MCC_DIO _NO_MCC_AO
}

DEFINES += LITTLE_ENDIAN
DEFINES += PRECOMPILED_HEADER
DEFINES += HAS_HFS

# Windows defines / libraries
win32 { DEFINES += WIN32 QWT_DLL }
win32 { DEFINES += _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 }
win32 { DEFINES += _CRT_SECURE_NO_WARNINGS }


win32 { LIBS += ../thirdparty/fftw/libfftw3-3.lib }
win32 { INCLUDEPATH += ../thirdparty/qwt-5.2/src }


#LINUX defines / libraries
unix { LIBS += /usr/lib/libqwt-qt4.so}
unix { LIBS += /usr/lib/libfftw3.so }
unix { INCLUDEPATH += /usr/include/qwt-qt4 }

DEFINES +=  RECURSIVE_QT_LOCKS

OTHERS += ../issues.txt


sharedSRC = ../shared/src
srcFPGA = ../IonizerES
sharedFPGA = $$srcFPGA/shared/src

INCLUDEPATH += $$sharedFPGA $$sharedSRC


INCLUDEPATH += $$shared $$sharedSRC

# Input
HEADERS += aluminizer.pro local.pro ../issues.txt

HEADERS += $$sharedFPGA/messaging.h  
HEADERS += $$sharedFPGA/dds_pulse_info.h  
HEADERS += $$sharedSRC/InputParameters.h $$sharedSRC/CriticalSection.h $$sharedSRC/RS232device.h $$sharedSRC/data_plot.h $$sharedSRC/ionizer_utils.h
HEADERS += common.h InputParametersModel.h InputParametersGUI.h ParameterGUI_Base.h Widgets.h AluminizerApp.h
HEADERS += AluminizerPage.h ExperimentsSheet.h SwitchPanelDialog.h DigitalOut.h 
HEADERS += TxtParametersGUI.h ExperimentPage.h MotorsPage.h FPGA_GUI.h
HEADERS += Experiment.h  RunObject.h ScanScheduler.h about.h
HEADERS += AnalogOutput.h AnalogOutParameter.h MCC_AnalogOut.h Voltages2.h Bfield.h
HEADERS += CommonExperimentsSheet.h GlobalsPage.h Histogram.h histogram_plot.h histogram_item.h
HEADERS += MgAlExperimentsSheet.h scan_variable.h TransitionPage.h AlignmentPage.h
HEADERS += ExpAl.h FrequencySource.h
HEADERS += Fitting.h ScanObject.h ExpSCAN.h ExpSCAN_Al.h plotlib/lm_lsqr.h
HEADERS += detection_stats.h DDS_Pulse_Widget.h
HEADERS += FPGA_TCP.h AgilisMotors.H
HEADERS += DDSPage.h FPGA_page.h FPGA_connection.h ScanSource.h ExpSCAN_NewFPGA.h ExpMM.h MotionPage.h ExpLMS.h
HEADERS += MgPage.h Ovens.h MatrixWidget.h Al3P1Page.h 
HEADERS += RefcavityPage.h RemoteFrequencySource.h $$sharedSRC/freq_control_packet.h
HEADERS += ExpZeroB.h DopplerTempCalibration.h CalibrationPage.h ExpCorrelate.h
HEADERS += $$sharedFPGA/Transition.h $$sharedFPGA/HFS.h $$sharedFPGA/HFS_Be.h \
                   $$sharedFPGA/HFS_Al.h $$sharedFPGA/HFS_Mg.h $$sharedFPGA/physics.h \
                   $$sharedFPGA/Numerics.h $$sharedFPGA/fractions.h $$sharedFPGA/string_func.h $$sharedFPGA/my_matrix.h



SOURCES += $$sharedFPGA/dds_pulse_info.cpp  
SOURCES += $$sharedSRC/InputParameters.cpp $$sharedSRC/CriticalSection.cpp $$sharedSRC/RS232device.cpp $$sharedSRC/data_plot.cpp $$sharedSRC/ionizer_utils.cpp
SOURCES += InputParametersGUI.cpp InputParametersModel.cpp main.cpp ParameterGUI_Base.cpp Widgets.cpp AluminizerApp.cpp
SOURCES += ExperimentsSheet.cpp SwitchPanelDialog.cpp DigitalOut.cpp MotorsPage.cpp
SOURCES += TxtParametersGUI.cpp ExperimentPage.cpp AluminizerPage.cpp FPGA_GUI.cpp
SOURCES += AnalogOutParameter.cpp MCC_AnalogOut.cpp Voltages2.cpp Bfield.cpp  AlignmentPage.cpp
SOURCES += Experiment.cpp  RunObject.cpp ScanScheduler.cpp about.cpp
SOURCES += GlobalsPage.cpp Histogram.cpp histogram_plot.cpp histogram_item.cpp
SOURCES += MgAlExperimentsSheet.cpp scan_variable.cpp TransitionPage.cpp
SOURCES += FrequencySource.cpp DDS_Pulse_Widget.cpp
SOURCES += Fitting.cpp FitRabi.cpp FitRamsey.cpp FitLine.cpp FitRepump.cpp plotlib/lm_lsqr.cpp ScanObject.cpp LockScan.cpp
SOURCES += DataFeed.cpp ExpSCAN.cpp ScanSource.cpp AgilisMotors.cpp
SOURCES += detection_stats.cpp StringConversion.cpp
SOURCES += FPGA_TCP.cpp GbE_msg.cpp
SOURCES += DDSPage.cpp FPGA_page.cpp FPGA_connection.cpp ExpSCAN_NewFPGA.cpp ExpSCAN_Al.cpp ExpLMS.cpp
SOURCES += MotionPage.cpp DopplerTempCalibration.cpp CalibrationPage.cpp
SOURCES += MgPage.cpp Ovens.cpp MatrixWidget.cpp Al3P1Page.cpp ExpCorrelate.cpp
SOURCES += RefcavityPage.cpp RemoteFrequencySource.cpp $$sharedSRC/freq_control_packet.cpp
SOURCES += $$sharedFPGA/HFS.cpp $$sharedFPGA/HFS_Mg.cpp $$sharedFPGA/HFS_Al.cpp $$sharedFPGA/physics.cpp \
           $$sharedFPGA/Numerics.cpp $$sharedFPGA/fractions.cpp  $$sharedFPGA/Transition.cpp \
           $$sharedFPGA/my_matrix.cpp $$sharedFPGA/string_func.cpp

OTHER += local.pro
