TEMPLATE = app
TARGET = LaserBrothers
CONFIG += console
QT += core \
    network
shared = ../../shared/src
thirdparty = ../../thirdparty

INCLUDEPATH += $$shared
DEFINES += NO_MCC
win32 { 
    CONFIG += debug_and_release
    CONFIG += build_all static
    INCLUDEPATH += $$thirdparty/gsl-1.8/include \
        $$thirdparty/qwt-5.2/src \
        $$thirdparty/NI
    DEFINES += WIN32 NO_NIDAQ
    LIBS += $$thirdparty/NI/NIDAQmx.lib \
        $$thirdparty/gsl-1.8/LIB/LIBGSL.A \
        $$thirdparty/gsl-1.8/LIB/LIBGSLCBLAS.A \
        qwt5.lib
    
    # LIBS += ../lib/cbw32.lib
    HEADERS += LaserBrothers.pro \
        $$shared/win_io.h
    SOURCES += $$shared/win_io.cpp
}
unix { 
    LIBS += /usr/lib/libqwt-qt4.so \
        /usr/lib/libcomedi.so \
        /usr/lib/libgsl.so \
        /usr/lib/libgslcblas.so
    INCLUDEPATH += /usr/include/qwt-qt4 \
        /usr/include/gsl
    DEFINES += NO_NIDAQ \
        NO_MCC HAS_COMEDI_ADIO
    HEADERS += $$shared/comedi_io.h
    SOURCES += $$shared/comedi_io.cpp
}
RC_FILE = resources.rc

HEADERS += data_plot.h \
    LockerWidget.h \
    NP_adjuster.h \
    WavePlate_adjuster.h \
    NelderMead_opt.h \
    $$shared/analog_io.h \
    $$shared/RS232device.h \
    actuator_opt.h \
    quadratic_opt.h \
    $$shared/CmdLineArgs.h \
    $$shared/InputParameters.h \
    $$shared/CriticalSection.h \
    adio.h
SOURCES += data_plot.cpp \
    LockerWidget.cpp \
    NP_adjuster.cpp \
    WavePlate_adjuster.cpp \
    NelderMead_opt.cpp \
    $$shared/analog_io.cpp \
    $$shared/RS232device.cpp \
    main.cpp \
    actuator_opt.cpp \
    quadratic_opt.cpp \
    $$shared/CmdLineArgs.cpp \
    $$shared/InputParameters.cpp \
    $$shared/CriticalSection.cpp \
    $$shared/string_func.cpp \
    adio.cpp
DEFINES += _CRT_SECURE_NO_WARNINGS \
    QWT_DLL
OTHER_FILES += local.pro
