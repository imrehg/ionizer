#pragma once

#include "ExperimentPage.h"
#include "Experiment.h"
#include "FrequencySource.h"
//#include "DDSsweeper.h"
#include "ExpAl.h"
#include "freq_control_packet.h"

class RemoteFrequencySource;

//! Page that communicates via signals/slots with a RemoteFrequencySource.  Signals are queued to avoid communication delays.
class RefCavityPage : public ParamsPage
{
Q_OBJECT

public:
RefCavityPage(const std::string& sPageName, ExperimentsSheet* pSheet, RemoteFrequencySource* pSynth);
virtual ~RefCavityPage();

//! returns the current actual frequency of the cavity compensation AOM
double GetCurrentFrequency();

//! return the current drift rate
double GetDriftRate();

//! Shift the *visible* light going towards the ion by delta_f Hz.  delta_f > 0 means blue shift
void shiftVisFrequency(double);

//! Shift the UV light going towards the ion by delta_f Hz delta_f > 0 means blue shift
void shiftPulseFrequency(double);

//! Send new error signal to cavity compensation program (under development)
void newErrSig(double err_sig);

virtual void PostCreateGUI();

//! clear history on remote compensator
void ClearHistory();

protected:
virtual void AddAvailableActions(std::vector<std::string>* v);
virtual void on_action(const std::string& s);
void setRFSState();

protected slots:
void slot_rcv_rfs_state(rfs_state);

signals:
void sig_set_rfs_state(rfs_state);
void sig_get_rfs_state();
void sig_shiftVisFrequency(double);
void sig_newErrSig(double);
void sig_connect();
void sig_clearHistory();

protected:
virtual bool RecalculateParameters();

RemoteFrequencySource* pSynth;

protected:
GUI_bool SetRemoteState;
GUI_double CompensationFreq, DriftRate, DriftWindow, DriftTimeConst;
GUI_double GainP, GainI, GainDrift;
GUI_double Shift;
GUI_string Server;
GUI_int Port;
GUI_string StatusText;
GUI_double MinCorrection;

double accumulated_correction;
};


#include "RemoteFrequencySource.h"

#ifdef _FPGA_NO_CARD
#include "../CavityCompensator/SimSynth.h"
#endif

class ElsnerPage : public RefCavityPage
{
public:
ElsnerPage(ExperimentsSheet* pSheet);

virtual ~ElsnerPage()
{
}

protected:

#ifdef _FPGA_NO_CARD
SimSynth AlFNRef;
#else
//Fiber noise reference between laser and Elsner
RemoteFrequencySource AlFNRef;
#endif
};

class ULE88Page : public RefCavityPage
{
public:
ULE88Page(ExperimentsSheet* pSheet);

virtual ~ULE88Page()
{
}

protected:

#ifdef _FPGA_NO_CARD
SimSynth Al2Pass;
#else
// 2-pass AOM between laser and ref. cavity
RemoteFrequencySource Al2Pass;
#endif
};


