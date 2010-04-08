TEMPLATE = app
TARGET = AluminizerSim

include(sim_local.pro)

INCLUDEPATH += ../INCLUDE/TNT ../INCLUDE/JAMA

FPGApath = ../IonizerES
shared = $$FPGApath/shared/src
DRIVER_PATH = ../FPGA/customIP2/IP/drivers/pulse_controller_v3_01_a/src
INCLUDEPATH += $$shared $$DRIVER_PATH $$FPGApath


CONFIG += console
DEFINES += ALUMINIZER_SIM LITTLE_ENDIAN _CRT_SECURE_NO_WARNINGS HAS_HFS CONFIG_PC

QT += core \
    network
HEADERS += AluminizerSim.pro sim_local.pro
HEADERS += $$FPGApath/info_interface.h
HEADERS += $$DRIVER_PATH/pulse_controller.h
HEADERS += $$FPGApath/common.h
HEADERS += $$FPGApath/remote_params.h
HEADERS += $$FPGApath/dac.h
HEADERS += $$shared/dds_pulse_info.h
HEADERS += $$FPGApath/dds_pulse.h
HEADERS += $$FPGApath/ttl_pulse.h
HEADERS += $$FPGApath/experiments.h
HEADERS += $$FPGApath/dacI.h $$FPGApath/voltagesI.h
HEADERS += $$FPGApath/motors.h
HEADERS += $$FPGApath/detection_stats.h $$FPGApath/FluorescenceChecker.h
HEADERS += $$FPGApath/exp_results.h
HEADERS += $$shared/messaging.h
HEADERS += simulator.h sim_ions.h
HEADERS += $$FPGApath/exp_recover.h
HEADERS += $$FPGApath/host_interface.h
HEADERS += $$shared/Transition.h $$shared/HFS.h \
                   $$shared/HFS_Al.h $$shared/HFS_Mg.h $$shared/physics.h \
                   $$shared/Numerics.h $$shared/fractions.h \
                   $$shared/my_matrix.h  $$shared/string_func.h
HEADERS += $$FPGApath/transition_info.h $$FPGApath/exp_correlate.h $$FPGApath/fft.h


SOURCES += $$FPGApath/ProcessMessage.cpp \
    $$FPGApath/info_interface.cpp
SOURCES += $$shared/dds_pulse_info.cpp
SOURCES += $$FPGApath/dds_pulse.cpp
SOURCES += $$FPGApath/experiments.cpp
SOURCES += $$FPGApath/dacI.cpp $$FPGApath/voltagesI.cpp
SOURCES += $$FPGApath/host_interface.cpp
SOURCES += $$FPGApath/motors.cpp $$FPGApath/exp_results.cpp
SOURCES += $$FPGApath/detection_stats.cpp $$FPGApath/FluorescenceChecker.cpp
SOURCES += $$shared/HFS.cpp $$shared/HFS_Mg.cpp $$shared/HFS_Al.cpp $$shared/physics.cpp \
           $$shared/Numerics.cpp $$shared/fractions.cpp $$shared/Transition.cpp $$shared/my_matrix.cpp $$shared/string_func.cpp
SOURCES += $$FPGApath/transition_info.cpp $$FPGApath/remote_params.cpp $$FPGApath/exp_correlate.cpp $$FPGApath/fft.cpp


SOURCES += main.cpp \
    simulator.cpp \
    sim_pulse_controller.cpp sim_dac.cpp \
    sim_ions.cpp
SOURCES += $$FPGApath/exp_recover.cpp
SOURCES += ../Ionizer/GbE_msg.cpp
AL { 
    DEFINES += CONFIG_AL
    HEADERS += $$FPGApath/Al/config_Al.h  $$FPGApath/Al/voltagesAl.h
    HEADERS += $$FPGApath/Al/exp_GSC.h
    HEADERS += $$FPGApath/Al/exp_Al3P1.h
    HEADERS += $$FPGApath/Al/exp_Al3P0.h \
                           $$FPGApath/Al/transition_info_Al.h \
                           $$FPGApath/Al/transition_info_Mg.h
			   
    SOURCES += $$FPGApath/Al/config_Al.cpp $$FPGApath/Al/voltagesAl.cpp
    SOURCES += $$FPGApath/Al/exp_GSC.cpp
    SOURCES += $$FPGApath/Al/exp_Al3P1.cpp
    SOURCES += $$FPGApath/Al/exp_Al3P0.cpp \
                           $$FPGApath/Al/transition_info_Al.cpp \
                           $$FPGApath/Al/transition_info_Mg.cpp
}
HG { 
    DEFINES += CONFIG_HG
    HEADERS += $$FPGApath/exp_LMS.h $$FPGApath/calcLMS.h $$FPGApath/Cordic.h
    SOURCES += $$FPGApath/Hg/config_Hg.cpp
    SOURCES += $$FPGApath/exp_LMS.cpp $$FPGApath/calcLMS.cpp $$FPGApath/Cordic.c
}
SIMPLE { 
    DEFINES += CONFIG_SIMPLE
    SOURCES += $$FPGApath/Hg/config_simple.cpp
}
win32:DEFINES += WIN32 \
    _CRT_SECURE_NO_WARNINGS}
