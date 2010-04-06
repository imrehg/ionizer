#ifndef WAVEPLATE_ADJUSTER_H
#define WAVEPLATE_ADJUSTER_H

//! Adjusts a pair of waveplates to maximize power

#include <QTime>
#include <QWidget>
#include <QSpinBox>

#include <string>
#include "RS232device.h"
#include "NelderMead_opt.h"


class WavePlate_adjuster : public QObject, public RS232device, public opt_actuator
{
	Q_OBJECT

public:
	WavePlate_adjuster(QWidget* parent, const std::string& port, unsigned baud);
    ~WavePlate_adjuster();

	int getAngle(unsigned iChannel);
	int shiftAngle(unsigned iChannel, int delta, bool bUpdateGUI);
	void setAngle(unsigned iChannel, int a);

	void newData(double signal, bool bFault);
	
	virtual void shift(const std::vector<double>& dx);

	bool isMoving();

protected:
	std::string cmd;

protected slots:
	void slot_updateAngle0(int);
	void slot_updateAngle1(int);

public:
	QTime tLastGood, tBad;
    QSpinBox wp0, wp1;
	QSpinBox* wp[2];

	int current_angles[2];
	NelderMead_opt nm_opt;
	QTime lastMovementT[2];
	int lastMovementS[2];
};

double restrict(double v, double bottom, double top);

#endif //WAVEPLATE_ADJUSTER_H