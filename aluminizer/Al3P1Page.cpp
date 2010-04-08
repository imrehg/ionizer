#include "common.h"

#include <math.h>

#include "Al3P1Page.h"
#include "HFS_Al.h"
#include "ExpAl.h"
#include "RefCavityPage.h"
#include "FPGA_connection.h"
#include "histogram_plot.h"
#include "ExpSCAN.h"

using namespace std;
using namespace physics;
using namespace numerics;


Al3P1Page* ExpAl::pAl3P1;
Al3P0Page* ExpAl::pAl3P0;

Al3P1Page::Al3P1Page(const string& sPageName,
                     ExperimentsSheet* pSheet,
                     unsigned page_id,
                     int num_sb) :
	TransitionPage(sPageName, pSheet, page_id,
	               (new HFS_Al_II_SingletS0()),
	               (new HFS_Al_II_TripletP1(3.5)),
	               "Al3P1",
	               num_sb, 3),

//	pRefCavity(dynamic_cast<RefCavityPage*>(m_pSheet->FindPage("ULE88"))),
	CommonModeCorrection("Common mode correction", &m_TxtParams, "0.5", &m_vParameters),
	NominalAOMCenter("Nominal AOM center [MHz]", &m_TxtParams, "", &m_vParameters)
{
	AOMorder = -1;
	ExpAl::pAl3P1 = this;
}

void Al3P1Page::InitExperimentStart()
{
	if (calibrations.empty())
	{
		//fill calibrations vector
		string calibration_type = "FULL";
		line l(mFg_min(), mFe_min(), 2);

		if (calibration_type == "FULL")
		{
			//calibrate sigma+ STR
			line l = line(mFg_max(), mFe_max(), 2);
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );


			//calibrate sigma+ carrier
			l = line(mFg_max(), mFe_max(), 0);
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );

			//calibrate sigma- STR
			l = line(mFg_min(), mFe_min(), 2);
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );

			//calibrate sigma- carrier
			l = line(mFg_min(), mFe_min(), 0);
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );
		}
	}
}



/*
   void Al3P1Page::AdjustBfield(double dB, double B0)
   {
   //shift only carriers for now
   int sideband = 0;

   double B_old = CalibrateMagneticField();
   double B_new = B_old * (1 + dB / B0);

   MagneticField.SetValue(B_new);

   for(double mFg = mFg_min(); mFg <= mFg_max(); ++mFg)
      for(double mFe = mFg - 1; mFe <= mFg + 1; ++mFe)
      {
         line l(mFg, mFe, sideband);
         if(tx.IsTransitionLegal(l))
         {
            double f = AOMFrequency(l, true);
            SetAOMFrequency(f, l);
         }
      }
   }
 */

double Al3P1Page::CalibrateMagneticField()
{
	double delta_m = mFe_max() > mFg_max() ? 1 : 0;

	line l1(mFe_min() + fabs(delta_m), mFe_min());
	line l2(mFe_max() - fabs(delta_m), mFe_max());

	double f1 = AOMFrequency(l1, false);
	double f2 = AOMFrequency(l2, false);


	if (isnan(f1) || isnan(f2))
		return 0;
	else
		return tx.CalibrateFrequency(l1, l2, f1, f2);
}

double Al3P1Page::ReferenceStateG(const physics::line& l)
{
	if (l.mFg == l.mFe)
		return -1 / 2.;
	else
		return mFg_min();
}

double Al3P1Page::ReferenceStateE(const physics::line& l)
{
	if (l.mFg == l.mFe)
		return ReferenceStateG(l);
	else
	{
		if (mFg_min() > mFe_min())
			return mFe_min();
		else
			return mFg_min() + 1;
	}
}

void Al3P1Page::ShiftLine(const physics::line& l1, double delta_f1, int)
{
/*
   update frequencies after measurement.
   For the clock where we depend on two transitions and take their average,
   we need to split the correction into cavity corrections, which shift
   both transitions together, and magnetic field corrections, which shift
   the line splitting

   If the magnetic field were perfectly constant, and the cavity was the only thing changing,
   we would have CommonModeCorrection = 0.999 or so.

   If the cavity were perfectly constant, and the magnetic field was the only thing changing,
   we would have CommonModeCorrection = -0.999 or so.


 */

	double f1 = AOMFrequency(l1);

	line l2 = l1;

	l2.InvertAngularMomentum();
	double f2 = AOMFrequency(l2);

	SetAOMFrequency(f1 + delta_f1, l1);
	SetAOMFrequency(f2 + CommonModeCorrection * delta_f1, l2);

#ifndef _FPGA_NO_CARD
//	ReCenterUVAOM(l1, l2, iUpdate);
#endif //_FPGA_NO_CARD
}

double Al3P1Page::ModeAmplitude(int sb)
{
	//TODO: check what this does.  seems incorrect for Mg/Al
	if (sb == 1)
	{
		double omegaZ = GetModeFrequency(sb) * 2 * M_PI;
		return sqrt(hbar / (2 * mass() * omegaZ) );
	}
	else
		throw runtime_error("[Al3P1Page::ModeAmplitude] bad mode number");
}

bool Al3P1Page::CanLink(const string& pname)
{
	return pname.find("Al3P1") == 0;
}

ExpSCAN* Al3P1Page::getCalibrationExp(const calibration_item&)
{
	return dynamic_cast<ExpSCAN*>( m_pSheet->FindExperiment("3P1 cal") );
}

Al3P0Page::Al3P0Page(const string& sPageName,
                     ExperimentsSheet* pSheet,
                     unsigned page_id,
                     int num_sb) :
	TransitionPage(sPageName, pSheet, page_id,
	               (new HFS_Al_II_SingletS0()),
	               (new HFS_Al_II_TripletP0()),
	               "Al3P0",
	               num_sb, 2),

	currentClockState(0),
	pRefCavity(dynamic_cast<RefCavityPage*>(m_pSheet->FindPage("Elsner"))),
	CommonModeCorrection("Common mode correction", &m_TxtParams, "0.5", &m_vParameters),
	NominalAOMCenter("Nominal AOM center [MHz]", &m_TxtParams, "", &m_vParameters),
	last_hist_update(0)
{
	AOMorder = 1;
	ExpAl::pAl3P0 = this;
}

//shift the cavity compensation so that the transition pair is centered around "center"
//this function should probably go somewhere else
//center is in AOM Hz.
void Al3P0Page::ReCenterUVAOM(const physics::line& l1,
                              const physics::line& l2,
                              bool bAdjustDrift)
{
	//difference between desired and actual center frequency in AOM Hz
	double delta_f = NominalAOMCenter * 1e6 - (AOMFrequency(l1) + AOMFrequency(l2)) / 2;

	// the UV AOM has order AOMorder (-2 when this code was written)
	// if delta_f = 1 we want the UV to be more blue by 2 Hz
	// i.e. the visible light to be more blue by 1 Hz
	pRefCavity->shiftVisFrequency( -AOMorder * delta_f / 2);

	if (!bAdjustDrift)
		pRefCavity->ClearHistory();

	ShiftAllFrequencies(delta_f);
}


double Al3P0Page::CalibrateMagneticField()
{
	double delta_m = mFe_max() > mFg_max() ? 1 : 0;

	line l1(mFe_min() + fabs(delta_m), mFe_min());
	line l2(mFe_max() - fabs(delta_m), mFe_max());

	double f1 = AOMFrequency(l1, false);
	double f2 = AOMFrequency(l2, false);


	if (isnan(f1) || isnan(f2))
		return 0;
	else
		return tx.CalibrateFrequency(l1, l2, f1, f2);
}

double Al3P0Page::ReferenceStateG(const physics::line&)
{
	return -5 / 2.;
}

double Al3P0Page::ReferenceStateE(const physics::line&)
{
	return -5 / 2.;
}

void Al3P0Page::ShiftLine(const physics::line& l1, double delta_f1, int iUpdate)
{
/*
        update frequencies after measurement.
        For the clock where we depend on two transitions and take their average,
        we need to split the correction into cavity corrections, which shift
        both transitions together, and magnetic field corrections, which shift
        the line splitting

        If the magnetic field were perfectly constant, and the cavity was the only thing changing,
        we would have CommonModeCorrection = 0.999 or so, i.e. if we shift the line for one state pair by x,
        the line for the opposite state pair should shift be almost the same amount.

        If the cavity were perfectly constant, and the magnetic field was the only thing changing,
        we would have CommonModeCorrection = -0.999 or so, i.e. if we shift the line for one state pair by x,
        the line for the opposite state pair should shift be almost the opposite amount.

        For CommonModeCorrection = 0, shifting one line won't move the other.
 */

	double f1 = AOMFrequency(l1);

	line l2 = l1;

	l2.InvertAngularMomentum();
	double f2 = AOMFrequency(l2);

	SetAOMFrequency(f1 + delta_f1, l1);
	SetAOMFrequency(f2 + CommonModeCorrection * delta_f1, l2);

#ifndef _FPGA_NO_CARD
	ReCenterUVAOM(l1, l2, iUpdate > 0);
#endif //_FPGA_NO_CARD
}

double Al3P0Page::ModeAmplitude(int sb)
{
	//TODO: check what this does.  seems incorrect for Mg/Al
	if (sb == 1)
	{
		double omegaZ = GetModeFrequency(sb) * 2 * M_PI;
		return sqrt(hbar / (2 * mass() * omegaZ) );
	}
	else
		throw runtime_error("[Al3P0Page::ModeAmplitude] bad mode number");
}

bool Al3P0Page::CanLink(const string& pname)
{
	return pname.find("Al3P0") == 0;
}

void Al3P0Page::update_histograms(double min_delay)
{
	if (det_hist_plot.size() == 0)
		AddHistograms();

	if (min_delay + last_hist_update > CurrentTime_s())
		return;

	ExperimentPage::pFPGA->SendParams(page_id, FPGA_params);

	for (unsigned i = 0; i < det_hist_plot.size(); i++)
	{
		if (det_hist_plot[i])
		{
			last_hist_update = CurrentTime_s();

			valarray<double> h = pFPGA->getDetectionHistogram(i);
			valarray<double> hs(20);

			for (size_t j = 0; j < hs.size(); j++)
				hs[j] = h[j];

			det_hist_plot[i]->Plot(hs);
		}
	}
}

void Al3P0Page::AddHistograms()
{
	unsigned c = 0;
	unsigned w = num_columns() / 2;

	for (unsigned j = 0; j < 2; j++)
	{
		hgrids.push_back(new QHBoxLayout());
		grid.addLayout(hgrids.back(), grid.rowCount(), 0, 4, -1);

		for (unsigned i = 0; i < 2; i++)
		{
			det_hist_plot.push_back(new histogram_plot(this, "", ""));
			hgrids.back()->addWidget(det_hist_plot.back());
			c += w;
			det_hist_plot.back()->show();
		}
	}
}

void Al3P0Page::on_action(const std::string& s)
{
	if (s == "CLEAR HIST")
	{
		pFPGA->resetStats();
		update_histograms(0);
	}

	if (s == "GET HIST")
		update_histograms(0);

	if (s == "RE-CENTER")
	{
		physics::line l1(-2.5, -2.5);
		physics::line l2(2.5, 2.5);

		ReCenterUVAOM(l1, l2, false);
	}


	TransitionPage::on_action(s);
}


void Al3P0Page::AddAvailableActions(std::vector<std::string>* v)
{
	TransitionPage::AddAvailableActions(v);

	v->push_back("CLEAR HIST");
	v->push_back("GET HIST");
	v->push_back("RE-CENTER");
}

ExpSCAN* Al3P0Page::getCalibrationExp(const calibration_item&)
{
	return dynamic_cast<ExpSCAN*>( m_pSheet->FindExperiment("3P0 cal") );
}
