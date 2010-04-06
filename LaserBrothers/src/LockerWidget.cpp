#include "LockerWidget.h"

//#define _USE_MATH_DEFINES
#include <math.h>

#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include <QSpacerItem>

#include "adio.h"
#include "NP_adjuster.h"
#include "WavePlate_adjuster.h"

using namespace std;

class Background: public QwtPlotItem
{
public:
	Background() : bGood(false), pmGood("nsmb_wp7_800.jpg"), pmBad("nsmb_wp2_800.jpg")
    {
        setZ(0.0);
    }

    virtual int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    virtual void draw(QPainter *painter,
        const QwtScaleMap &, const QwtScaleMap &,
        const QRect &rect) const
    {

		if(bGood)
			painter->drawPixmap(rect, pmGood);
		else
			painter->drawPixmap(rect, pmBad);

    }

	bool bGood;
	QPixmap pmGood, pmBad;
};


string get_name(unsigned id)
{
	char buff[64];
	snprintf(buff, 64, "Locker %u", id);
	return string(buff);
}

LockerWidget::LockerWidget(QWidget* parent, unsigned id, InputParameters* params,
								   adio* ad, QSettings* settings, QWidget* legendParent) :
QGroupBox("", parent),
ad(ad),
name(get_name(id) + " name", params, "Locker " + to_string<int>(id)),
name_str(get_name(id)),
adjWP_path(name_str + string(" waveplate adjuster"), params, "none"),
adjNP_path(name_str + string(" NP adjuster path") , params, "none"),
adjNP_cmd(name_str +  " NP adjuster command", params, "T1"),
Tmin(name_str + " NP adjuster Tmin", params, "50"),
Tmax(name_str + " NP adjuster Tmax", params, "50"),
ai_dev(name_str + " AI_device index", params, "0"),
ai_chan(name_str + " AI_channel index", params, "0"),
ao_dev(name_str + " AO_device index", params, "0"),
ao_chan(name_str + " AO_channel index", params, "0"),
do_dev(name_str + " DO_device index", params, "0"),
do_chan(name_str + " DO_channel index", params, "0"),
dependsOn(name_str + " dependency", params, "-1"),
monitor_gain(name_str + " monitor gain", params, "1"),
minSweep(name_str + " min sweep", params, "-2.49"),
maxSweep(name_str + " max sweep", params, "2.49"),
grid(this),
lbl_max_mon("Max. cavity (V)", parent),
lbl_cav_sig("Cavity signal (V)", parent),
lbl_min_mon("Min. cavity (V)", parent),
lbl_delta("Scan delta", parent),
lbl_T("Laser T", parent),
max_mon(parent),
min_mon(parent),
scan_delta(parent),
T(parent),
cav_sig(parent),
run_mode("Run mode", parent),
mode_auto("Automatic re-lock", &run_mode), 
mode_scan("Scan", &run_mode), 
mode_integrator_on("Enable integrator", &run_mode), 
mode_integrator_off("Disable integrator", &run_mode),
shiftT("Servo laser T", parent),
optWaveplates("Optimize wave plates", parent),
alwaysReplot("Continuous plot", parent),
plot(parent),
sweepOutput(0),
sweepOutputLastLocked(0),
run_state(ACQUIRE),
scan_sign(1),
adjNP(0),
adjWP(0),
bFault(true),
dependsOnLW(0),
numReplots(0),
settings(settings),
bg(0)
{
	setTitle(name.Value().c_str());
	setFlat(false); //draw a frame

	if(adjWP_path.Value() != "none")
		adjWP = new WavePlate_adjuster(this, adjWP_path, 921600);

	if(adjNP_path.Value() != "none")
		adjNP = new NP_adjuster(adjNP_path, 57600, adjNP_cmd, Tmin, Tmax, 0.01);


	setDO(true); //disable intgrator

//	bg = new Background();
//   bg->attach(&plot);

	max_mon.setRange(-10,10);
	max_mon.setDecimals(3);
	min_mon.setRange(-10,10);
	min_mon.setDecimals(3);
	scan_delta.setRange(-1,1);
	scan_delta.setDecimals(3);

	T.setDecimals(3);
	T.setSingleStep(0.01);
	T.setRange(0,100);

	alwaysReplot.setToolTip("Always update plots (checked), or only when unlocked (unchecked)");
	restoreSettings(settings);

	shiftT.setChecked(false);
	  
	cavityMonitorInput = 0;
	AcqIndex = 0;

	mode_auto.setChecked(true);

	unsigned r = 0; //row number of next widget
	grid.addWidget(&lbl_max_mon,r,0);
	grid.addWidget(&max_mon,r++,1);

	grid.addWidget(&lbl_cav_sig,r,0);
	grid.addWidget(&cav_sig,r++,1);
	cav_sig.setReadOnly(true);
        cav_sig.setMaximumWidth(100);

	grid.addWidget(&lbl_min_mon,r,0);
	grid.addWidget(&min_mon,r++,1);

	grid.addWidget(&lbl_delta,r,0);
	grid.addWidget(&scan_delta,r++,1);

    if(adjNP)
    {
        grid.addWidget(&lbl_T,r,0);
        grid.addWidget(&T,r++,1);
        QObject::connect(&T, SIGNAL(valueChanged(double)),
                  this, SLOT(setLaserT(double)));
    }
    else
    {
        T.hide();
        lbl_T.hide();
    }

	grid.addWidget(&run_mode,r,0,2,2);
	r += 2;

	if(adjWP)
	{
        grid.addWidget(&optWaveplates,r++,0);
		grid.addWidget(adjWP->wp[0],r,0);
		grid.addWidget(adjWP->wp[1],r++,1);
	}
	else
	{
		optWaveplates.hide();
	}

    grid.addWidget(&shiftT,r++,0);
    grid.addWidget(&alwaysReplot,r++,0);

	grid.addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), r++, 0);

	if(adjNP)
	{
		shiftT.setChecked(true);
	}
	else
	{
		shiftT.setChecked(false);
		shiftT.setDisabled(true);
	}


        grid.addWidget(&plot, 0, 2, r, 1);
        grid.setColumnStretch(2, 6);
        grid.setColumnStretch(1, 0.5);

	QVBoxLayout* vl = new QVBoxLayout(&run_mode);
	vl->addWidget(&mode_auto);
	vl->addWidget(&mode_scan);
	vl->addWidget(&mode_integrator_on);
	vl->addWidget(&mode_integrator_off);


	// Disable polygon clipping
    QwtPainter::setDeviceClipping(false);

	QPalette pal;
	pal.setColor(QPalette::Window, Qt::white);
	plot.setPalette(pal);

    // We don't need the cache here
    plot.canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    plot.canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

	std::vector<QwtPlotCurve*> curves;

	//  Initialize data

    for (int i = 0; i< PLOT_SIZE; i++)
    {
        d_x[i] = 0.5 * i;     // time axis

		for(int j=0;j<4;j++)
			d_y[j][i] = 0.0+0.1*j;
    }

    // Insert new curves
        curves.push_back( new QwtPlotCurve("Cavity sig.") );
	curves.back()->setPen(QPen(Qt::red));

	curves.push_back( new QwtPlotCurve("Servo") );
	curves.back()->setPen(QPen(Qt::blue));

	curves.push_back( new QwtPlotCurve("Fault") );
	curves.back()->setPen(QPen(Qt::green));

	curves.push_back( new QwtPlotCurve("Sweep") );
	curves.back()->setPen(QPen(Qt::black));
	curves.back()->setStyle(QwtPlotCurve::Dots);


	// Axis 
    plot.setAxisTitle(QwtPlot::xBottom, "Time");
    plot.setAxisScale(QwtPlot::xBottom, 0, 100);
    plot.setAxisScale(QwtPlot::yLeft, -5.0, 5.0);

    alignScales();

    if(legendParent)
    {
        plot.insertLegend(new QwtLegend(legendParent), QwtPlot::ExternalLegend );
        plot.legend()->setGeometry(500,15,400,50);
    }

	for(unsigned i=0; i<curves.size(); i++)
	{
		curves[i]->attach(&plot);

		// Attach (don't copy) data. All curves use the same x array.
		curves[i]->setRawData(d_x, &(d_y[i][0]), PLOT_SIZE);
	}

	palStop = max_mon.palette();
        palGood = min_mon.palette();

   //     for(unsigned i=0; i<20; i++)
        {
            palGood.setColor((QPalette::Text), Qt::darkGreen);
            palBad.setColor((QPalette::Text), Qt::red);
        }

	tLastUnlock.start();
}

LockerWidget::~LockerWidget()
{
	settings->beginGroup(name.Value().c_str());
	settings->setValue("Max. cavity", max_mon.value());
	settings->setValue("Min. cavity", min_mon.value());
	settings->setValue("Scan delta", scan_delta.value());
	settings->setValue("Shift T", shiftT.isChecked());
	settings->setValue(alwaysReplot.text(), alwaysReplot.isChecked());
	settings->endGroup();

	if(adjWP)
		delete adjWP;
}

int LockerWidget::dependency()
{
	return dependsOn;
}

void LockerWidget::restoreSettings(QSettings* s)
{
	s->beginGroup(name.Value().c_str());

	if(s->contains("Max. cavity"))
		max_mon.setValue(s->value("Max. cavity").toDouble());
	else
		max_mon.setValue(2);

	if(s->contains("Min. cavity"))
		min_mon.setValue(s->value("Min. cavity").toDouble());
	else
		min_mon.setValue(1);

	if(s->contains("Scan delta"))
		scan_delta.setValue(s->value("Scan delta").toDouble());
	else
		scan_delta.setValue(0.01);

	if(s->contains(alwaysReplot.text()))
		alwaysReplot.setChecked(s->value(alwaysReplot.text()).toBool());
	else 
		alwaysReplot.setChecked(true);

	s->endGroup();
}

//
//  Set a plain canvas frame and align the scales to it
//

void LockerWidget::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    plot.canvas()->setFrameStyle(QFrame::Box | QFrame::Plain );
    plot.canvas()->setLineWidth(1);

    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)plot.axisWidget(i);
        if ( scaleWidget )
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)plot.axisScaleDraw(i);
        if ( scaleDraw )
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
}

void LockerWidget::replot()
{
	numReplots++;

	if(getFault())
		numReplots = 0;

	if( (!alwaysReplot.isChecked()) && (numReplots > 5))
		return;

	

	if(nInCavSigTotal)
		cav_sig.setText(QString("%1").arg(cav_sig_total/nInCavSigTotal,6,'f',4));

	cav_sig_total = 0;
	nInCavSigTotal = 0;

	plot.replot();
}

void LockerWidget::setLaserT(double)
{
    if(adjNP)
        adjNP->setTemperature(T.value());
}

void LockerWidget::TurnIntOn()
{
	bFault = false;
	if(bg) bg->bGood = true;
	setDO(bFault);
}

void LockerWidget::stop()
{
	TurnIntOff();
	
	min_mon.setPalette(palStop);
	max_mon.setPalette(palStop);
}

void LockerWidget::TurnIntOff()
{
	bFault = true;
	if(bg) bg->bGood = false;
	numReplots = 0;

	setDO(bFault);
}

void LockerWidget::processNewData()
{
	cavityMonitorInput = get_cavity_signal();
	servoMonitorInput = get_servo_signal();
}


//! Calculate sweep output (AuxOut) to re-acquire the lock or scan across the resonance
double LockerWidget::calcSweepOut()
{	
	if(run_state == ACQUIRE)
	{
		tLastUnlock.restart();

		//don't change anything if the locker upon which this depends is unlocked
		if(dependsOnLW)
			if(dependsOnLW->getFault())
				return sweepOutput;

		double amplitude = (0.1+(double)AcqIndex/3000.0);
		double min_out = sweepOutputLastLocked - amplitude;
		double max_out = sweepOutputLastLocked + amplitude;

		sweepOutput += scan_sign * scan_delta.value();

		if(sweepOutput > max_out)
		{
			scan_sign *= -1;
			sweepOutput = max_out;
		}

		if(sweepOutput < min_out)
		{
			scan_sign *= -1;
			sweepOutput = min_out;
		}

		AcqIndex++;
	}
	else
	{
		AcqIndex = 0;

		if(run_state == SCAN)
		{
			sweepOutput += 100*scan_sign * scan_delta.value();

			if(sweepOutput > maxSweep)
			{
				scan_sign *= -1;
				sweepOutput = maxSweep;
			}

			if(sweepOutput < minSweep)
			{
				scan_sign *= -1;
				sweepOutput = minSweep;
			}

		}
		else
			if(run_state == LOCKED)
			{
				//drive servo output towards zero
				if(servoMonitorInput > 0.1)
					sweepOutput += scan_delta.value();

				if(servoMonitorInput < -0.1)
					sweepOutput -= scan_delta.value();

				if(adjNP && shiftT.isChecked()) //tune temperature
				{
                                        //if last unlock was > 10 s ago, and last adjustment > 10 s ago
                                        if(adjNP->timeOfLastAdj.elapsed() > 10000 && tLastUnlock.elapsed() > 10000)
					{
						try
						{
							if(sweepOutput > 0.5)
                                                                adjNP->shiftTemperatureUp();

							if(sweepOutput < -0.5)
                                                                adjNP->shiftTemperatureDown();
						}
						catch (runtime_error e) {}
					}
				}
			}
			else
				sweepOutput = sweepOutputLastLocked;
	}

	if(sweepOutput>maxSweep || sweepOutput<minSweep)
	{
		sweepOutputLastLocked = 0; //is this correct?
		sweepOutput = 0;
		AcqIndex = 0;
	}

	sweepOutput = restrict(sweepOutput, minSweep, maxSweep);

	return sweepOutput;
}

void LockerWidget::updateIntegratorStatus()
{
    if(adjNP)
        T.setValue(adjNP->getTemperature());

	if(mode_integrator_on.isChecked())
	{
		TurnIntOn();
	}
	else
	{
		if(mode_integrator_off.isChecked())
		{
			TurnIntOff();
		}
		else
		{
			if(run_state == LOCKED)
			{
				TurnIntOn();	
			}
			else
			{
				if(run_state == ACQUIRE)
				{
					TurnIntOff();
				}
			}
		}
	}
}

void LockerWidget::updateRunState()
{
	RUN_STATE new_run_state;

	//update GUI colors
	if(cavityMonitorInput >= min_mon.value())
		min_mon.setPalette(palGood);
	else
		min_mon.setPalette(palBad);

	if(cavityMonitorInput <= max_mon.value())
		max_mon.setPalette(palGood);
	else
		max_mon.setPalette(palBad);

	//switch to WAIT if enable/disable integrator is set
	if(mode_integrator_on.isChecked())
		new_run_state = WAIT;

	if(mode_integrator_off.isChecked())
		new_run_state = WAIT;

	//in auto mode determine run state based on cavity monitor and window
	if(mode_auto.isChecked())
	{
		bool bDependencyOK =  true;

		if(dependsOnLW)
			bDependencyOK = ! dependsOnLW->getFault();

		if ( bDependencyOK && (cavityMonitorInput >= min_mon.value()) && (cavityMonitorInput <= max_mon.value()) )
		{
			//acquired the lock
			new_run_state = LOCKED;

			//turn off acquire
			AcqIndex = 0;
			sweepOutputLastLocked = sweepOutput;
		}
		else
			new_run_state = ACQUIRE;
	}
	else
	{
		if(mode_scan.isChecked())
			new_run_state = SCAN;
	}

	//if run state changed, print info
	if(new_run_state != run_state)
	{
		string timestr(QTime::currentTime().toString().toStdString());
		run_state = new_run_state;
		cout << timestr << " " << setw(14) << name.Value() << " " << getStatusStr() <<  endl;
	}
}

const char* LockerWidget::getStatusStr()
{
	return toString(run_state);
}

const char* LockerWidget::toString(RUN_STATE r)
{
	switch(r) 
	{
	case SCAN	 : return "SCAN   ";
	case ACQUIRE : return "ACQUIRE";
	case LOCKED  : return "LOCKED ";
	case WAIT	 : return "WAIT   ";
	default		 : return "UNKNOWN";
	}
}


//! Determines the status of the lock.  Returns new analog_out data.
double LockerWidget::calculateOutput()
{
	updateRunState();
	updateIntegratorStatus();
	calcSweepOut();

	if(adjWP && optWaveplates.isChecked())
		adjWP->newData(cavityMonitorInput, getFault());

	setAO(sweepOutput);

	return sweepOutput;
}

double LockerWidget::getPlotChannel(unsigned i)
{
	switch(i)
	{
		case 0: return cavityMonitorInput*monitor_gain;
		case 1: return servoMonitorInput;
		case 2: return getFault() ? 5 : 0;
		case 3: return sweepOutput;
		default: return 0;
	}
}

void LockerWidget::updatePlotData()
{
	cav_sig_total += cavityMonitorInput;
	nInCavSigTotal++;

	for(unsigned j=0; j<4; j++)
	{
		//shift data backwards...this is very inefficient
		 for ( int i = 0; i < PLOT_SIZE - 1; i++ )
			d_y[j][i] = d_y[j][i+1];

		d_y[j][PLOT_SIZE - 1] = getPlotChannel(j);
	}
}

bool LockerWidget::getFault()
{
	return bFault;
}

std::string LockerWidget::getName()
{
	return name;
}

void LockerWidget::addDependency(LockerWidget* lw)
{
	dependsOnLW = lw;
}

void LockerWidget::setDO(bool b)
{
	ad->set_do(do_dev, do_chan, b);
}

void LockerWidget::setAO(double d)
{
	ad->set_ao(ao_dev, ao_chan, d);
}

double LockerWidget::get_cavity_signal()
{
	return ad->get_ai(ai_dev, ai_chan);
}

double LockerWidget::get_servo_signal()
{
	return ad->get_ai(ai_dev, ai_chan+1);
}
