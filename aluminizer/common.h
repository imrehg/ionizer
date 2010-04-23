#ifndef COMMON_H
#define COMMON_H

#define NOMINMAX
#define _USE_MATH_DEFINES

#ifdef WIN32

#define snprintf _snprintf
#define hypot _hypot
#define isnan _isnan

#else

typedef unsigned DWORD;
typedef unsigned UINT;
typedef char* LPTSTR;

#endif

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <iomanip>
#include <valarray>
#include <memory>
#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <time.h>
#include <math.h>
#include <assert.h>

extern double g_t0;
extern std::string g_t0s;

#include <trlib.h>
#include "string_func.h"

#include <QApplication>
#include <QDir>
//#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QCheckBox>
#include <QProgressBar>
#include <QComboBox>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QGridLayout>
#include <QWaitCondition>
#include <QTextEdit>
#include <QSound>
#include <QColorDialog>

#include <QAction>
#include <QThread>
#include <QTemporaryFile>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QProcess>
#include <QMutex>

#endif //__cplusplus

bool debugQ(const std::string& sCategory, const std::string& sMsg);

#endif // COMMON_H

