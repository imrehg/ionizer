#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExpAl.h"
#include "GlobalsPage.h"
#include "Al3P1Page.h"
#include "RefCavityPage.h"

using namespace std;
using namespace numerics;

RefCavityPage::RefCavityPage(const std::string& sPageName, ExperimentsSheet* pSheet, RemoteFrequencySource* pSynth) :
	ParamsPage(pSheet, sPageName),
	pSynth(pSynth),
	SetRemoteState("Set remote state",       &m_TxtParams, "false", &m_vParameters),
	CompensationFreq("AOM Freq. [Hz]",         &m_TxtParams, "0", &m_vParameters),
	DriftRate("Drift rate [Hz/s]",      &m_TxtParams, "0.0", &m_vParameters, true),
	DriftWindow("Drift window [s]",    &m_TxtParams, "1000", &m_vParameters),
	DriftTimeConst("Drift time const. [s]",     &m_TxtParams, "120", &m_vParameters),
	GainP("P gain [Hz/err_sig]",    &m_TxtParams, "1", &m_vParameters),
	GainI("I gain [Hz/sum(err_sig)]", &m_TxtParams, "1", &m_vParameters),
	GainDrift("Drift gain [(Hz/s)/sum(err_sig)]", &m_TxtParams, "1", &m_vParameters),
	Shift("Shift [Hz]",          &m_TxtParams, "5.0", &m_vParameters),
	Server("Server",               &m_TxtParams, "847neuron", &m_vParameters),
	Port("Port",              &m_TxtParams, "5001", &m_vParameters),
	StatusText("Status",              &m_TxtParams, "", &m_vParameters, true),
	MinCorrection("Minimum correction",     &m_TxtParams, "0", &m_vParameters),
	accumulated_correction(0)
{
	StatusText.setFlag(RP_FLAG_NOPACK | RP_FLAG_READ_ONLY, true);
	SetRemoteState.SetValue(false);
	MinCorrection.setToolTip("Minimum correction to accumulate before sending a frequency update");

//	SafeRecalculateParameters();
}

RefCavityPage::~RefCavityPage()
{
}

void RefCavityPage::PostCreateGUI()
{
	ParamsPage::PostCreateGUI();

	pSynth->start();

	QObject::connect(this, SIGNAL(sig_shiftVisFrequency(double)),
	                 pSynth, SLOT(slot_shiftVisFrequency(double)),
	                 Qt::QueuedConnection);

	QObject::connect(this, SIGNAL(sig_set_rfs_state(rfs_state)),
	                 pSynth, SLOT(slot_set_state(rfs_state)),
	                 Qt::QueuedConnection);

	QObject::connect(this, SIGNAL(sig_get_rfs_state()),
	                 pSynth, SLOT(slot_get_state()),
	                 Qt::QueuedConnection);

	QObject::connect(this, SIGNAL(sig_newErrSig(double)),
	                 pSynth, SLOT(slot_newErrSig(double)),
	                 Qt::QueuedConnection);

	QObject::connect(this, SIGNAL(sig_connect()),
	                 pSynth, SLOT(slot_connect()),
	                 Qt::QueuedConnection);

	QObject::connect(this, SIGNAL(sig_clearHistory()),
	                 pSynth, SLOT(slot_clearHistory()),
	                 Qt::QueuedConnection);

	QObject::connect(pSynth, SIGNAL(sig_new_state(rfs_state)),
	                 this, SLOT(slot_rcv_rfs_state(rfs_state)),
	                 Qt::QueuedConnection);

	emit sig_connect();
}

void RefCavityPage::slot_rcv_rfs_state(rfs_state rfs)
{
	if (!SetRemoteState.Value())
	{
		CompensationFreq.SetValue( rfs.freq_or_delta );
		DriftRate.SetValue( rfs.ramp_rate );
		DriftWindow.SetValue( rfs.ramp_window );
		DriftTimeConst.SetValue( rfs.ramp_time_const );

		emit sig_update_data();
	}
}

void RefCavityPage::AddAvailableActions(std::vector<std::string>* v)
{
	AluminizerPage::AddAvailableActions(v);

	v->push_back("UPDATE");
	v->push_back("SHIFT +");
	v->push_back("SHIFT -");

	if (pSynth)
		v->push_back("CONNECT");
}


void RefCavityPage::on_action(const std::string& s)
{
	if (s == "UPDATE")
		emit sig_get_rfs_state();

	if (s == "SHIFT +")
		emit sig_shiftVisFrequency(Shift);

	if (s == "SHIFT -")
		emit sig_shiftVisFrequency(-1 * Shift);

	if (s == "CONNECT")
		emit sig_connect();
}



bool RefCavityPage::RecalculateParameters()
{

	bool Changed = false;

	try
	{
		CompensationFreq.SetReadOnly( !SetRemoteState );
		DriftRate.SetReadOnly( !SetRemoteState );
		DriftWindow.SetReadOnly( !SetRemoteState );
		DriftTimeConst.SetReadOnly( !SetRemoteState );

		if (SetRemoteState)
			setRFSState();
	}
	catch (runtime_error) {}

	Changed |= StatusText.SetValue(pSynth->getStatusText());


	return Changed;
}

void RefCavityPage::setRFSState()
{
	rfs_state rfs;

	rfs.freq_or_delta = CompensationFreq;
	rfs.ramp_rate = DriftRate;
	rfs.ramp_window = DriftWindow;
	rfs.ramp_time_const = DriftTimeConst;

	emit sig_set_rfs_state(rfs);
}

void RefCavityPage::shiftPulseFrequency(double delta_f)
{
	shiftVisFrequency(delta_f * 0.5);
}

void RefCavityPage::shiftVisFrequency(double delta_f)
{
	accumulated_correction += delta_f;

	if (fabs(accumulated_correction) >= MinCorrection.Value())
	{
		emit sig_shiftVisFrequency(accumulated_correction);
		accumulated_correction = 0;
	}
}

void RefCavityPage::newErrSig(double err_sig)
{
	emit sig_newErrSig(err_sig);
}

void RefCavityPage::ClearHistory()
{
	emit sig_clearHistory();
}

double RefCavityPage::GetCurrentFrequency()
{
	if ( pSynth)
		return pSynth->GetEstimatedFrequency();

	else return 0;
}

double RefCavityPage::GetDriftRate()
{
	if ( pSynth)
		return pSynth->GetRampRate();
	else
		return 0;
}


ElsnerPage::ElsnerPage(ExperimentsSheet* pSheet) :
	RefCavityPage("Elsner", pSheet, &AlFNRef),
	AlFNRef(80e6, 90e6, -1, "FN synth", Server, Port)
{
	CompensationFreq.setRange(AlFNRef.dMinFreq, AlFNRef.dMaxFreq);
}

ULE88Page::ULE88Page(ExperimentsSheet* pSheet) :
	RefCavityPage("ULE88", pSheet, &Al2Pass),
	Al2Pass(-1e9, 1e9, -1, "2Pass synth", Server, Port)
{
	Al2Pass.SweepTo(CompensationFreq);
}




