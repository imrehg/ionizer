#pragma once

#ifndef PRECOMPILED_HEADER
#include <QObject>
#include <QLabel>
#include <QAction>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#endif

#include "dds_pulse_info.h"
#include "physics.h"


class scan_target
{
public:
scan_target() : axis(0)
{
}

scan_target(const std::string& label, const std::string& scan_type, int axis = 0) :
	label(label), scan_type(scan_type), axis(axis)
{
}

std::string label;
std::string scan_type;
int axis;
};

class Pulse_Widget;
class TTL_Pulse_Widget;
class DDS_Pulse_Widget;
class DDS_SB_Pulse_Widget;

class TransitionPage;

class Pulse_Widget : public QWidget
{
Q_OBJECT

public:
Pulse_Widget(QWidget* parent = 0, Qt::WindowFlags f = 0);
~Pulse_Widget();

void setName(const std::string&);
const std::string& getName()
{
	return name;
}

void updateEnabled(bool b);

virtual double getPulseTime() = 0;
virtual void setTransitionPage(TransitionPage* pX);


signals:
void valueChanged();
void setup_scan(scan_target);
void setup_t_cal(Pulse_Widget*);
void sig_update_ss(QWidget *, QString);    //update widget background to indicate scanning

public slots:
void slot_setup_t_scan();
void slot_setup_t_cal();
void slot_update_ss(QWidget *, QString);

protected slots:
void slot_valueChanged();
void slot_disable();

protected:
void enableValChangeSig(bool b);

std::string name;
std::string initial_name;
QHBoxLayout layout;
QLabel label_t;
QString ssScanning, ssNonScanning;

public:
QAction action_disable, action_t_scan, action_t_cal;

protected:
TransitionPage* pX;
//  physics::line l;

QPalette palEnabled, palDisabled, palScanning, palNonScanning;    //UI colors

public:
QDoubleSpinBox dsb_t;

bool bEnabled;
bool bValChangeSig;
};

class TTL_Pulse_Widget : public Pulse_Widget
{
public:
TTL_Pulse_Widget(QWidget* parent = 0, Qt::WindowFlags f = 0);
virtual ~TTL_Pulse_Widget();

ttl_pulse_info getPulseInfo() const;
void setPulseInfo(const ttl_pulse_info&);

void selectScanWidget(const std::string& scan_type, bool bScan);

virtual double getPulseTime();
};

class DDS_Pulse_Widget : public Pulse_Widget
{
Q_OBJECT
public:
DDS_Pulse_Widget(QWidget* parent = 0, double freq_unit = 1e6, Qt::WindowFlags f = 0);
virtual ~DDS_Pulse_Widget();

dds_pulse_info getPulseInfo() const;
void setPulseInfo(const dds_pulse_info&);

void disable_fOff();

virtual double getPulseTime();
virtual void setTransitionPage(TransitionPage* pX);
void selectScanWidget(const std::string& scan_type, bool bScan);

signals:
void setup_f_cal(Pulse_Widget*);

public slots:
void slot_setup_f_scan();
void slot_setup_f_cal();

protected:
QLabel label_fOn, label_fOff;
public:
QAction action_f_scan, action_f_cal;
public:
QDoubleSpinBox dsb_fOn, dsb_fOff;

double freq_unit;
};
