#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "DDS_Pulse_Widget.h"
#include "TransitionPage.h"

Pulse_Widget::Pulse_Widget(QWidget* parent, Qt::WindowFlags f) :
	QWidget(parent, f),
	label_t("t = "),
	ssScanning("background-color: rgb(150, 170, 255); border-color: rgb(150, 170, 255);"),
	ssNonScanning(""),
	action_disable("Disable", this),
	action_t_scan("scan", this),
	action_t_cal("cal", this),
	pX(0),
	bEnabled(true),
	bValChangeSig(true)
{
	setAutoFillBackground(true);

	palDisabled.setColor(QPalette::Window, QColor(255, 170, 155));
	palEnabled = palette();

	palScanning.setColor(QPalette::Base, QColor(150, 170, 255));
	palNonScanning = dsb_t.palette();

	label_t.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout.setContentsMargins(0, 0, 0, 0);

	layout.addWidget(&label_t);
	layout.addWidget(&dsb_t);

	addAction(&action_disable);
	setContextMenuPolicy(Qt::ActionsContextMenu);

	dsb_t.setSuffix(" us");
	dsb_t.setRange(0.0, 1000000);
	dsb_t.setDecimals(3);

	dsb_t.setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
	dsb_t.addAction(&action_t_scan);
	dsb_t.addAction(&action_t_cal);
	dsb_t.setContextMenuPolicy(Qt::ActionsContextMenu);
	dsb_t.setKeyboardTracking(false);


	ssNonScanning = dsb_t.styleSheet();

	QObject::connect(&dsb_t, SIGNAL(valueChanged(double)), this, SLOT(slot_valueChanged()), Qt::AutoConnection);
	QObject::connect(&action_t_scan, SIGNAL(triggered(bool)), this, SLOT(slot_setup_t_scan()), Qt::AutoConnection);
	QObject::connect(&action_t_cal, SIGNAL(triggered(bool)), this, SLOT(slot_setup_t_cal()), Qt::AutoConnection);
	QObject::connect(&action_disable, SIGNAL(triggered(bool)), this, SLOT(slot_disable()), Qt::AutoConnection);
	QObject::connect(this, SIGNAL(sig_update_ss(QWidget *, QString)), this, SLOT(slot_update_ss(QWidget *, QString)), Qt::AutoConnection);

	setLayout(&layout);
	show();
}

Pulse_Widget::~Pulse_Widget()
{
}


void Pulse_Widget::slot_update_ss(QWidget* w, QString ss)
{
	w->setStyleSheet(ss);
}


void Pulse_Widget::setTransitionPage(TransitionPage* pX)
{
	this->pX = pX;

	QObject::connect(this, SIGNAL(setup_t_cal(Pulse_Widget*)), pX, SLOT(slot_cal_t(Pulse_Widget*)), Qt::AutoConnection);
}


void Pulse_Widget::setName(const std::string& s)
{
	name = s;

	if (initial_name.length() == 0)
		initial_name = name;

}

void Pulse_Widget::enableValChangeSig(bool b)
{
	bValChangeSig = b;
}

void Pulse_Widget::slot_valueChanged()
{
	if (bValChangeSig)
		emit valueChanged();
}


void Pulse_Widget::slot_setup_t_scan()
{
	debugQ("[Pulse_Widget::slot_setup_scan]", getName() + " (t-scan)");

	scan_target st(initial_name + "(T)", "Time");
	emit setup_scan(st);
}

void Pulse_Widget::updateEnabled(bool b)
{
	bEnabled = b;
	action_disable.setText(bEnabled ? "Disable" : "Enable");
	setPalette(b ? palEnabled : palDisabled);
}

void Pulse_Widget::slot_disable()
{
	updateEnabled(!bEnabled);

	emit valueChanged();
}

void Pulse_Widget::slot_setup_t_cal()
{
	emit setup_t_cal(this);
}

DDS_Pulse_Widget::DDS_Pulse_Widget(QWidget* parent, double freq_unit, Qt::WindowFlags f) :
	Pulse_Widget(parent, f),
	label_fOn("fOn = "),
	label_fOff("fOff = "),
	action_f_scan("scan", this),
	action_f_cal("cal", this),
	freq_unit(freq_unit)
{
	label_fOn.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout.addWidget(&label_fOn);
	layout.addWidget(&dsb_fOn);

	label_fOff.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout.addWidget(&label_fOff);
	layout.addWidget(&dsb_fOff);
	dsb_fOff.setSuffix(" MHz");
	dsb_fOff.setRange(-2000, 2000);
	dsb_fOff.setDecimals(0);
	dsb_fOff.setKeyboardTracking(false);

	dsb_fOn.setRange(-2e9 / freq_unit, 2e9 / freq_unit);
	dsb_fOn.setKeyboardTracking(false);

	if (freq_unit == 1e6)
	{
		dsb_fOn.setSuffix(" MHz");
		dsb_fOn.setDecimals(6);
	}

	if (freq_unit == 1e3)
	{
		dsb_fOn.setSuffix(" kHz");
		dsb_fOn.setDecimals(3);
	}

	if (freq_unit == 1)
	{
		dsb_fOn.setSuffix(" Hz");
		dsb_fOn.setDecimals(3);
	}

	dsb_fOn.setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
	dsb_fOff.setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);

	dsb_fOn.addAction(&action_f_scan);
	dsb_fOn.addAction(&action_f_cal);
	dsb_fOn.setContextMenuPolicy(Qt::ActionsContextMenu);

	QObject::connect(&dsb_fOn, SIGNAL(valueChanged(double)), this, SLOT(slot_valueChanged()), Qt::AutoConnection);
	QObject::connect(&dsb_fOff, SIGNAL(valueChanged(double)), this, SLOT(slot_valueChanged()), Qt::AutoConnection);

	QObject::connect(&action_f_scan, SIGNAL(triggered(bool)), this, SLOT(slot_setup_f_scan()), Qt::AutoConnection);
	QObject::connect(&action_f_cal, SIGNAL(triggered(bool)), this, SLOT(slot_setup_f_cal()), Qt::AutoConnection);
}

DDS_Pulse_Widget::~DDS_Pulse_Widget()
{
}

void DDS_Pulse_Widget::selectScanWidget(const std::string& scan_type, bool bScan)
{
	if (scan_type == "Frequency")
		emit sig_update_ss(&dsb_fOn, bScan ? ssScanning : ssNonScanning);

	if (scan_type == "Time")
		emit sig_update_ss(&dsb_t, bScan ? ssScanning : ssNonScanning);
}

void DDS_Pulse_Widget::setTransitionPage(TransitionPage* pX)
{
	Pulse_Widget::setTransitionPage(pX);
	QObject::connect(this, SIGNAL(setup_f_cal(Pulse_Widget*)), pX, SLOT(slot_cal_f(Pulse_Widget*)), Qt::AutoConnection);
}

void DDS_Pulse_Widget::disable_fOff()
{
	label_fOn.setText("f = ");
	label_fOff.hide();
	dsb_fOff.hide();
}

dds_pulse_info DDS_Pulse_Widget::getPulseInfo() const
{
	//if there is a scan running, substitute in the scan's value

	dds_pulse_info i;

	i.t = dsb_t.value();
	i.fOn = dsb_fOn.value() * freq_unit / 1e6;
	i.fOff = dsb_fOff.value();
	i.setEnabledFlag(bEnabled);

	return i;
}

double DDS_Pulse_Widget::getPulseTime()
{
	return dsb_t.value();
}


void DDS_Pulse_Widget::setPulseInfo(const dds_pulse_info& i)
{
	dds_pulse_info iOld = getPulseInfo();

	if (iOld != i)
	{
		enableValChangeSig(false);

		dsb_t.setValue(i.t);
		dsb_fOn.setValue(i.fOn * 1e6 / freq_unit);
		dsb_fOff.setValue(i.fOff);
		updateEnabled(i.getEnabledFlag());

		enableValChangeSig(true);

		//     emit valueChanged();
	}
}

void DDS_Pulse_Widget::slot_setup_f_scan()
{
	debugQ("[Pulse_Widget::slot_setup_scan]", getName() + " (f-scan)");

	scan_target st(initial_name + "(F)", "Frequency");
	emit setup_scan(st);
}

void DDS_Pulse_Widget::slot_setup_f_cal()
{
	emit setup_f_cal(this);
}

TTL_Pulse_Widget::TTL_Pulse_Widget(QWidget* parent, Qt::WindowFlags f) :
	Pulse_Widget(parent, f)
{
}

TTL_Pulse_Widget::~TTL_Pulse_Widget()
{
}


void TTL_Pulse_Widget::selectScanWidget(const std::string& scan_type, bool bScan)
{
	if (scan_type == "Time")
		emit sig_update_ss(&dsb_t, bScan ? ssScanning : ssNonScanning);
}

ttl_pulse_info TTL_Pulse_Widget::getPulseInfo() const
{
	ttl_pulse_info i;

	i.t = dsb_t.value();
	i.setEnabledFlag(bEnabled);

	return i;
}

double TTL_Pulse_Widget::getPulseTime()
{
	return dsb_t.value();
}

void TTL_Pulse_Widget::setPulseInfo(const ttl_pulse_info& i)
{
	ttl_pulse_info iOld = getPulseInfo();

	if (iOld != i)
	{
		enableValChangeSig(false);

		dsb_t.setValue(i.t);
		updateEnabled(i.getEnabledFlag());

		enableValChangeSig(true);
		//    emit valueChanged();
	}
}
