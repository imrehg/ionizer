#pragma once

#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QTime>
#include <QSettings>
#include <QLineEdit>
#include <QDoubleSpinBox>

#include <qwt_counter.h>
#include <qwt_plot.h>

#include <fstream>

#include <InputParameters.h>

#define PLOT_SIZE  (201)      // 0 to 200

class NP_adjuster;
class WavePlate_adjuster;


//! Keeps lasers and doubling cavities locked

/*! 
HW requirements (per locker):
2 A/D inputs
1 D/A output
1 Digital output

Logic:

For NIST loop filters and HV amps:

Connect the CAVITY MONITOR input signal to an ADC channel.
Connect the LOOP FILTER MONITOR to another ADC channel.
Connect the HV amp SWEEP input to a DAC channel.
Connect the loop filter WINDOW input to a digital output channel.

For auto re-lock mode:

If the CAVITY MONITOR is between the min and max range from the GUI, the status switches to LOCKED.
Then the digital output (WINDOW in) goes low to enable the loop filter integrator.
The SWEEP signal gets adjusted slowly to drive LOOP FILTER MONITOR towards zero.
If "Set temperature" is enabled in the GUI, and an NP photonics fiber laser is connected,
its temperature gets adjusted slowly to drive SWEEP towards zero.

If the CAVITY MONITOR is not between the min and max range from the GUI, the status switches to ACQUIRE.
Then the digital (WINDOW in) goes high to disable the loop filter integrator.
The SWEEP signal gets swept slowly in a search pattern.
*/

class Background;
class adio;

class LockerWidget : public QGroupBox
{
	Q_OBJECT

public:
	LockerWidget(QWidget* parent, unsigned id, InputParameters* params,
				 adio* ad,
             QSettings* settings,
             QWidget* legendParent=0);

	virtual ~LockerWidget();

	//! return index >= 0 of locker that this one depends on
	int dependency();
	void addDependency(LockerWidget* lw);

	void processNewData();
	double calcSweepOut();

	void updatePlotData();
	void replot();

	//! Determines the status of the lock. 
	void updateRunState();

	//!  Returns new analog_out data.
	double calculateOutput();

	double getPlotChannel(unsigned);

	bool getFault();
	std::string getName();

	void TurnIntOn();
	void TurnIntOff();

	void stop();
	const char* getStatusStr();

	enum RUN_STATE {ACQUIRE, LOCKED, SCAN, WAIT};

 protected slots:
        void setLaserT(double);

protected:
	void restoreSettings(QSettings* s);
	void alignScales();
	void updateIntegratorStatus();
	const char* toString(RUN_STATE);

	void setDO(bool b);
	void setAO(double d);
	double get_cavity_signal();
	double get_servo_signal();

protected:
	adio* ad;
	param_string name;
	std::string name_str;
	param_string adjWP_path, adjNP_path, adjNP_cmd;
	param_double Tmin, Tmax;
	param_int ai_dev, ai_chan;
	param_int ao_dev, ao_chan;
	param_int do_dev, do_chan;
	param_int dependsOn;
	param_double monitor_gain, minSweep, maxSweep;

	QGridLayout grid;
    QLabel lbl_max_mon, lbl_cav_sig, lbl_min_mon, lbl_delta, lbl_T;
    QDoubleSpinBox max_mon, min_mon, scan_delta, T;
	QLineEdit cav_sig;

	QGroupBox run_mode;
	QRadioButton mode_auto, mode_scan, mode_integrator_on, mode_integrator_off;
	QCheckBox shiftT, optWaveplates, alwaysReplot;


    QwtPlot plot;
	
	int AcqIndex;

	double cavityMonitorInput, servoMonitorInput, sweepOutput, sweepOutputLastLocked;

	QPalette palGood, palBad, palStop;

	double d_x[PLOT_SIZE]; 
    double d_y[4][PLOT_SIZE];

	
	RUN_STATE run_state;

	int scan_sign;

	NP_adjuster* adjNP;
	WavePlate_adjuster* adjWP;
	bool bFault;
	LockerWidget* dependsOnLW;

	unsigned numReplots;

	double cav_sig_total;
	int nInCavSigTotal;

public:
	QTime tLastUnlock;
	QSettings* settings;
	Background* bg;
};
