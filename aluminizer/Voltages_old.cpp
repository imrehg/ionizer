#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif


#include "Voltages.h"
#include "NI_AnalogOut.h"
#include "MCC_AnalogOut.h"

#include "ExperimentPage.h"
#include "AluminizerApp.h"
#include "FPGA_GUI.h"
#include "FPGA_connection.h"

#include <tnt.h>
#include <jama_lu.h>

//#include "sphelper.h"

Voltages* gVoltages = 0;


VoltagesBase::VoltagesBase(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	FPGA_GUI(sPageName, pSheet, page_id)
{
}

void VoltagesBase::SetNamePrefix(const std::string& name)
{
	QWriteLocker lock(&page_lock);

	for (size_t i = 0; i < prefixed_params.size(); i++)
		prefixed_params[i]->setInternalNamePrefix(name);

}


Voltages::Voltages(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	VoltagesBase(sPageName, pSheet, page_id),
	twist_multiplier(1),
//tNI(2),
	ECratio("Endcaps ratio",       &m_TxtParams, "1", &m_vParameters),
	ECmean("Endcaps mean [V]",    &m_TxtParams, "0", &m_vParameters),
	ECtwist("Endcaps twist [V]",      &m_TxtParams, "0", &m_vParameters),

//ForceIdentity		("Identity matrix",				&m_TxtParams, "false", &m_vParameters),
	NoUpdates("No updates",          &m_TxtParams, "false", &m_vParameters),
	SettingsName("Settings",      "Run\nRun(low)\nLoad\nReorder(MgAl)\nReorder(AlMg)\nReorder(MgMgAl)\nReorder(MgAlMg)\nDump Heavy\nRun2\nRun3\n",  &m_TxtParams, "Run", &m_vParameters),
	ControlMode("Control mode",  "XY\nHV\n", &m_TxtParams, "HV", &m_vParameters),

	RampSteps("Ramp steps",              &m_TxtParams, "10", &m_vParameters),
	DwellTime("Dwell time [ms]",           &m_TxtParams, "16", &m_vParameters),
	CrystallizeReorder("Crystallize reorder",               &m_TxtParams, "false", &m_vParameters),
	FlipOrder("Flip order",              &m_TxtParams, "false", &m_vParameters),
	ReorderSteps("Reorder steps",              &m_TxtParams, "10", &m_vParameters),
	ReorderWait("Reorder wait [ms]",                &m_TxtParams, "10", &m_vParameters),
	ReorderPeriod("Reorder period [s]",               &m_TxtParams, "10", &m_vParameters),
	ReorderDir("Direction", "MgAl\nAlMg\nMgMgAl\nMgAlMg\n", &m_TxtParams, "MgAl", &m_vParameters),

	VcerfExy("V[CE,RF] for E[x,y]", &m_TxtParams, 2, 2, "(2x2)", &m_vParameters),
	VcerfEhv("V[CE,RF] for E[h,v]", &m_TxtParams, 2, 2, "(2x2)", &m_vParameters),

	Exy("Exy [V/cm]",        &m_TxtParams, 1, 2, "(1x2)", &m_vParameters),
	Ehv("Ehv [V/cm]",        &m_TxtParams, 1, 2, "(1x2)", &m_vParameters),

	cmCE("c.m. CE [V]",          &m_TxtParams, "0", &m_vParameters),
	cmRF("c.m. RF [V]",          &m_TxtParams, "0", &m_vParameters),

	guiV6("V6 [V]",           &m_TxtParams, "0", &m_vParameters),
	guiV7("V7 [V]",           &m_TxtParams, "0", &m_vParameters),
//V6low				("V6 low [V]",			&m_TxtParams, "0", &m_vParameters, false, false),

	M_ExyV(2, 2, 0.),
	M_EhvV(2, 2, 0.),
//voltage_log(("voltages_" + g_t0s + ".csv").c_str()),
	tLastReorder(0),
	sndCrystallize(soundDir.absoluteFilePath("smb3_1-up.wav")),
	sndDump(soundDir.absoluteFilePath("dk-a2600_die.wav")),
	flip_sign(1)
{
	FlipOrder.setToolTip("Flip order periodically");
	VcerfExy.forceNewRow();
	Exy.forceNewRow();
	cmCE.forceNewRow();

	QObject::connect(this, SIGNAL(trigger_action(QString)), this, SLOT(slot_on_action(QString)), Qt::AutoConnection);

	gVoltages = this;

	RampSteps.setRange(0, 10000);
	DwellTime.setRange(0, 1000);

	oldSettingsName = SettingsName;
	ECratio.setPrecision(6);

	for (int i = 0; i < 6; i++)
		g_scan_sources.push_back(new VScanSource(i, this));
/*
   double vMax = 10;
   double vMin = -10;

   ce[0]		= new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 0, -10, 10, 0.2465)), 0,
                           "CE LZ* [V]",	&m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

   ce[1]		= new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 1, -10, 10, 0.2465)), 0,
                           "CE LZ [V]",	&m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

   rf[0]		= new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 2, vMin, vMax)), 0,
                           "RF+ [V]",	&m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

   rf[1]		= new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 3, vMin, vMax)), 0,
                           "RF- [V]",	&m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

   endcaps1 = new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 5, vMin, vMax)), 0,
                           "Endcap 1 [V]",	&m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

   endcaps2a = new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 4, vMin, vMax)), 0,
                           "Endcap 2a [V]",	&m_TxtParams,  &m_vParameters, 0, 5, &voltage_log);

   endcaps2b = new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 7, vMin, vMax)), 0,
                           "Endcap 2b [V]",	&m_TxtParams,  &m_vParameters, 0, 5, &voltage_log);

   V6 = new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 6, 0, 1.4)), 0,
                           "V 6 [V]",	&m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);
 */

	double vMin = -29;
	double vMax = 49;

	ce[0]    = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 0, vMin, vMax)), 0,
	                                  "CE LZ* [V]",  &m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

	ce[1]    = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 1, vMin, vMax)), 0,
	                                  "CE LZ [V]",   &m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

	rf[0]    = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 2, vMin, vMax)), 0,
	                                  "RF+ [V]",  &m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

	rf[1]    = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 3, vMin, vMax)), 0,
	                                  "RF- [V]",  &m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);


	endcaps1 = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 4, 0, 800)), 0,
	                                  "EC LZ [V]",   &m_TxtParams,  &m_vParameters, 0, 1,  &voltage_log);

	endcaps2a = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5535", 0, 0, 200)), 0,
	                                   "ECc LZ [V]",  &m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

	endcaps2b = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5535", 1, 0, 200)), 0,
	                                   "ECc LZ* [V]", &m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

	V6 = new AnalogOutParameter(auto_ptr<AnalogOut>(new FPGA_AnalogOut(FPGA_GUI::pFPGA, "AD5370", 5, 0, 1.4)), 0,
	                            "V 6 [V]",  &m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

//	V6 = new AnalogOutParameter(auto_ptr<AnalogOut>(new NI_AnalogOut(&tNI, 6, 0, 1.4)), 0,
//									"V 6 [V]",	&m_TxtParams,  &m_vParameters, 0, 1, &voltage_log);

	prefixed_params.push_back(&ECratio);
	prefixed_params.push_back(&ECmean);
	prefixed_params.push_back(&ECtwist);
	prefixed_params.push_back(&Exy);
	prefixed_params.push_back(&Ehv);
	prefixed_params.push_back(&cmCE);
	prefixed_params.push_back(&cmRF);
	prefixed_params.push_back(&guiV6);

	SetNamePrefix(SettingsName);

	guiV6.setIncrement(0.002);

	endcaps1->SetReadOnly(true);
	endcaps2a->SetReadOnly(true);
	endcaps2b->SetReadOnly(true);

	for (int i = 0; i < 2; i++)
	{
		ce[i]->SetReadOnly(true);
		rf[i]->SetReadOnly(true);

		voltage_parameters.push_back(ce[i]);
		voltage_parameters.push_back(rf[i]);
	}

	voltage_parameters.push_back(endcaps1);
	voltage_parameters.push_back(endcaps2a);
	voltage_parameters.push_back(endcaps2b);
	voltage_parameters.push_back(V6);

	ExperimentPage::pVoltages = this;

	UpdateInverseMatrices();

	double a, b;

	SetExy(0, Exy.Value().element(0, 0), Exy.Value().element(0, 1), &a, &b, cmCE, cmRF);
	SetEndcaps(ECratio, ECmean, ECtwist, &a, &b);

	SafeRecalculateParameters();
}

void Voltages::UpdateInverseMatrices()
{
	GUI_matrix* VE[2] = { &VcerfExy, &VcerfEhv };

	TNT::Array2D<double>* EV[2] = { &M_ExyV, &M_EhvV };

	for (unsigned m = 0; m < 2; m++)
	{
		//LU-decompose VE
		TNT::Array2D<double> mVE(2, 2);
		CopyArray2D(VE[m]->Value(), &mVE);

		JAMA::LU<double> lu(mVE);;
		TNT::Array1D<double> V(2, 0.);

		//solve the system MVE*Ece = V
		V[0] = 1;
		V[1] = 0;
		TNT::Array1D<double> Ece = lu.solve(V);

		//solve the system MVE*Ece = V
		V[0] = 0;
		V[1] = 1;
		TNT::Array1D<double> Erf = lu.solve(V);

		if (Ece.dim1() && Erf.dim1())
		{
			for (int i = 0; i < 2; i++)
			{
				(*EV[m])[i][0] = Ece[i];
				(*EV[m])[i][1] = Erf[i];
			}
		}
	}
}


Voltages::~Voltages()
{
	while ( !voltage_parameters.empty() )
	{
		RemoveParameter(voltage_parameters.back());
		delete voltage_parameters.back();
		voltage_parameters.pop_back();
	}
}

void Voltages::PostCreateGUI()
{
	VoltagesBase::PostCreateGUI();

	Exy.input->setColLabel(0, "Ex");
	Exy.input->setColLabel(1, "Ey");

	Ehv.input->setColLabel(0, "Eh");
	Ehv.input->setColLabel(1, "Ev");

	VcerfExy.input->setRowLabel(0, "CE");
	VcerfExy.input->setRowLabel(1, "RF");
	VcerfExy.input->setColLabel(0, "Ex");
	VcerfExy.input->setColLabel(1, "Ey");

	VcerfEhv.input->setRowLabel(0, "CE");
	VcerfEhv.input->setRowLabel(1, "RF");
	VcerfEhv.input->setColLabel(0, "Eh");
	VcerfEhv.input->setColLabel(1, "Ev");
}

void Voltages::AddAvailableActions(std::vector<std::string>* p)
{
	VoltagesBase::AddAvailableActions(p);

	p->push_back("CRYSTALLIZE");
	p->push_back("RAMP DOWN");
	p->push_back("RAMP UP");

	p->push_back("MgAl");
	p->push_back("AlMg");
	p->push_back("MgMgAl");
	p->push_back("MgAlMg");
	p->push_back("DUMP");
	p->push_back("DUMP HEAVY");

}

void Voltages::on_action(const std::string& s)
{
	cout << "[Voltages::on_action] " << s << endl;

	if (s == "CRYSTALLIZE") Crystallize(-1, 1);

	if (s == "RAMP UP") Ramp(1);

	if (s == "RAMP DOWN") Ramp(-1);

	if (s.find("Mg") != string::npos)
		if (s.find("Al") != string::npos)
			Reorder(s);

	if (s == "DUMP") Dump();
	if (s == "DUMP HEAVY") Crystallize(-2, 1);

	VoltagesBase::on_action(s);
}


int Voltages::ReorderPeriodic(bool bForce)
{
	int dir = 0;

	if (bForce || (CurrentTime_s() - tLastReorder > ReorderPeriod))
	{
		tLastReorder = CurrentTime_s();


		//string s = "REORDER (" + ReorderDir.Value() +")";
		string s = ReorderDir.Value();

		if (FlipOrder)
		{
			if (flip_sign == -1)
			{
				if (strcmp(s.c_str(), "MgAl") == 0)
					s = "AlMg";
				else
					s = "MgAl";
			}

			flip_sign *= -1;
		}

		//Reorder(s);
		emit trigger_action(QString(s.c_str()));

		if (strcmp(s.c_str(), "MgAl") == 0)
			dir = 1;
		else
			dir = -1;
	}

	return dir;
}

void Voltages::Reorder(const std::string& voltage_setting)
{

	//lock FPGA during re-ordering so no experiments can run
	QWriteLocker fpga_locker(&(theApp->fpga->fpga_lock));
	QWriteLocker lock(&page_lock);

	string vs = voltage_setting;

	if (vs.length() == 0)
		vs = ReorderDir.Value();

	cout << "Reorder " << voltage_setting << endl;

//	string s = (order == 1) ? "Reorder(MgAl)" : "Reorder(AlMg)";
	string s = "Reorder(" + vs + ")";

	RampSettings("Run", s, ReorderSteps, 1);

	tLastReorder = CurrentTime_s();

	SleepHelper::msleep(ReorderWait);
}

void Voltages::Dump()
{
	sndDump.play();
	double v6old = V6->Value();

	V6->SetValue(0);
	UpdateAll();

	SleepHelper::msleep(100);

	V6->SetValue(v6old);
	UpdateAll();
}

void Voltages::Ramp(int direction)
{
	QWriteLocker lock(&page_lock);

	cout << "[Voltages::Ramp] entry" << endl;

	if (direction == -1)
	{
		sndCrystallize.play();
		RampSettings(oldSettingsName, "Run(low)", RampSteps);
		SettingsName.SetValue("Run(low)");
		oldSettingsName = SettingsName;
		SettingsName.PostUpdateGUI();
	}

	if (direction == -2)
	{
		RampSettings(oldSettingsName, "Dump Heavy", RampSteps);
		SettingsName.SetValue("Dump Heavy");
		oldSettingsName = SettingsName;
		SettingsName.PostUpdateGUI();
	}

	if (direction == 1)
	{
		RampSettings(oldSettingsName, "Run", RampSteps);
		SettingsName.SetValue("Run");
		oldSettingsName = SettingsName;
		SettingsName.PostUpdateGUI();
	}

	if (direction == 1 && CrystallizeReorder)
	{
		SleepHelper::msleep(ReorderWait);
		Reorder(ReorderDir);
	}

	cout << "[Voltages::Ramp] exit" << endl;
}

void Voltages::Crystallize(int dir1, int dir2)
{
	//lock FPGA during crystallize so no experiments can run
	QWriteLocker fpga_locker(&(theApp->fpga->fpga_lock));

	Ramp(dir1);
	SleepHelper::msleep(RampSteps * DwellTime);
	Ramp(dir2);

}

void Voltages::SetEndcaps(double ECratio_new, double ECmean_new, double ECtwist_new, double* ECratio_actual, double* ECmean_actual)
{
	QWriteLocker lock(&page_lock);

	endcaps1->SetValue(2 * ECmean_new / (1 + 1 / ECratio_new));

	endcaps2a->SetValue(2 * ECmean_new / (1 + ECratio_new) + 0.5 * ECtwist_new);
	endcaps2b->SetValue(2 * ECmean_new / (1 + ECratio_new) - 0.5 * ECtwist_new);

	//could be different from nominal due to voltage clipping
	*ECratio_actual = GetECratio();
	*ECmean_actual = GetECmean();

	ECratio.SetValue(*ECratio_actual);
	ECmean.SetValue(*ECmean_actual);
	ECtwist.SetValue(ECtwist_new);

	if (fabs(ECratio_new - *ECratio_actual) > 0.001)
		cout << "***WARNING*** [Voltages::SetEndcaps] ECratio_nominal = " << fixed << setprecision(6) << ECratio_new << "   ECdelta_actual = " << *ECratio_actual << " V" << endl ;
//		MessageBeep(MB_ICONEXCLAMATION);

	if (fabs(ECmean_new - *ECmean_actual) > 0.001)
		cout << "***WARNING*** [Voltages::SetEndcaps] ECmean_nominal = " << fixed << setprecision(3) << ECmean_new << "   ECmean_actual = " << *ECmean_actual << " V" << endl ;
//		MessageBeep(MB_ICONEXCLAMATION);
}

void Voltages::SetECmean(double d)
{
	double oldRatio = GetECratio();
	double a, b;

	SetEndcaps(oldRatio, d, ECtwist, &a, &b);
}

double Voltages::GetECmean()
{
	return ( endcaps1->Value() + 0.5 * (endcaps2a->Value() + endcaps2b->Value())) / 2;
}

void Voltages::SetECratio(double d)
{

	double oldMean = GetECmean();
	double a, b;

	SetEndcaps(d, oldMean, ECtwist, &a, &b);
}

double Voltages::GetECratio()
{
	return endcaps1->Value() / (0.5 * (endcaps2a->Value() + endcaps2b->Value()));
}

matrix_t Voltages::VEmatrix()
{
	if (ControlMode.Value() == "XY")
		return VcerfExy.Value();
	else
		return VcerfEhv.Value();
}

double Voltages::VExy(unsigned i, unsigned j)
{
	return VcerfExy.Value().element(i, j);
}

double Voltages::VEhv(unsigned i, unsigned j)
{
	return VcerfEhv.Value().element(i, j);

}

double Voltages::ExyV(unsigned i, unsigned j)
{
	return M_ExyV[i][j];
}

double Voltages::EhvV(unsigned i, unsigned j)
{
	return M_EhvV[i][j];
}

void Voltages::SetExy(unsigned mode, double Ex_new, double Ey_new, double* Ex_actual, double* Ey_actual,
                      double cmCE_new, double cmRF_new)
{
	QWriteLocker lock(&page_lock);

	matrix_t VE = mode == 0 ? VcerfExy : VcerfEhv;

	for (int i = 0; i < 2; i++)
	{
		double ce_new = (i - 0.5) * (VE.element(0, 0) * Ex_new  +  VE.element(0, 1) * Ey_new) + cmCE_new;
		double rf_new = (i - 0.5) * (VE.element(1, 0) * Ex_new  +  VE.element(1, 1) * Ey_new) + cmRF_new;
		ce[i]->SetValue(ce_new);
		rf[i]->SetValue(rf_new);
	}



	if (mode == 0)
	{
		//could be different from nominal due to voltage clipping
		*Ex_actual = GetEx();
		*Ey_actual = GetEy();
		Exy.SetElement(0, 0, *Ex_actual);
		Exy.SetElement(0, 1, *Ey_actual);

		UpdateEhv();
	}
	else
	{
		*Ex_actual = GetEh();
		*Ey_actual = GetEv();
		Ehv.SetElement(0, 0, *Ex_actual);
		Ehv.SetElement(0, 1, *Ey_actual);
		UpdateExy();
	}

	if (fabs(Ex_new - *Ex_actual) > 0.001)
		cout << "***WARNING*** [Voltages::SetExy] Ex_nominal = " << fixed << setprecision(3) << Ex_new << "   Ex_actual = " << *Ex_actual << " V/cm" << endl;
		//	MessageBeep(MB_ICONEXCLAMATION);
	if (fabs(Ey_new - *Ey_actual) > 0.001)
		cout << "***WARNING*** [Voltages::SetExy] Ey_nominal = " << fixed << setprecision(3) << Ey_new << "   Ey_actual = " << *Ey_actual << " V/cm" << endl ;
		//	MessageBeep(MB_ICONEXCLAMATION);

//	UpdateEhv();
}

void Voltages::SetEx(double Ex_new)
{
	QWriteLocker lock(&page_lock);

	double Ey0 = GetEy();
	double Exa, Eya;

	SetExy(0, Ex_new, Ey0, &Exa, &Eya, cmCE, cmRF);
}

void Voltages::SetEy(double Ey_new)
{
	QWriteLocker lock(&page_lock);

	double Ex0 = GetEx();
	double Exa, Eya;

	SetExy(0, Ex0, Ey_new, &Exa, &Eya, cmCE, cmRF);
}

void Voltages::SetEh(double Eh_new)
{
	QWriteLocker lock(&page_lock);

	double Ev0 = GetEv();
	double Eha, Eva;

	SetExy(1, Eh_new, Ev0, &Eha, &Eva, cmCE, cmRF);
}

void Voltages::SetEv(double Ev_new)
{
	QWriteLocker lock(&page_lock);

	double Eh0 = GetEh();
	double Eha, Eva;

	SetExy(1, Eh0, Ev_new, &Eha, &Eva, cmCE, cmRF);
}

double Voltages::GetEx()
{
	QWriteLocker lock(&page_lock);

	TNT::Array1D<double> V(2, 0.);
	V[0] = ce[1]->Value() - ce[0]->Value();
	V[1] = rf[1]->Value() - rf[0]->Value();

	double x = ExyV(0, 0) * V[0] + ExyV(0, 1) * V[1];

	return 0.00001 * floor(x * 100000 + 0.5);
}

double Voltages::GetEy()
{
	QWriteLocker lock(&page_lock);

	TNT::Array1D<double> V(2, 0.);
	V[0] = ce[1]->Value() - ce[0]->Value();
	V[1] = rf[1]->Value() - rf[0]->Value();

	double y =  ExyV(1, 0) * V[0] + ExyV(1, 1) * V[1];

	return 0.00001 * floor(y * 100000 + 0.5);
}

double Voltages::GetEh()
{
	QWriteLocker lock(&page_lock);

	TNT::Array1D<double> V(2, 0.);
	V[0] = ce[1]->Value() - ce[0]->Value();
	V[1] = rf[1]->Value() - rf[0]->Value();

	double x = EhvV(0, 0) * V[0] + EhvV(0, 1) * V[1];

	return 0.00001 * floor(x * 100000 + 0.5);
}

double Voltages::GetEv()
{
	QWriteLocker lock(&page_lock);

	TNT::Array1D<double> V(2, 0.);
	V[0] = ce[1]->Value() - ce[0]->Value();
	V[1] = rf[1]->Value() - rf[0]->Value();

	double y =  EhvV(1, 0) * V[0] + EhvV(1, 1) * V[1];

	return 0.00001 * floor(y * 100000 + 0.5);
}


void Voltages::RampSettings(const std::string& name0, const std::string& name1, unsigned num_steps, unsigned rampBack)
{
	QWriteLocker lock(&page_lock);

	vector<double> V0(8);
	vector<double> V1(8);

	vector<double>* pV[2];
	pV[0] = &V0;
	pV[1] = &V1;

	vector<string> names(2);
	names[0] = name0;
	names[1] = name1;

	for (size_t i = 0; i < 2; i++)
	{
		SetNamePrefix(names[i]);
		(*(pV[i]))[0] = ECratio.Value();
		(*(pV[i]))[1] = ECmean.Value();

		(*(pV[i]))[2] = Exy.Value().element(0, 0);
		(*(pV[i]))[3] = Exy.Value().element(0, 1);

		(*(pV[i]))[4] = cmCE.Value();
		(*(pV[i]))[5] = cmRF.Value();

		(*(pV[i]))[6] = guiV6.Value();
		(*(pV[i]))[7] = ECtwist.Value();
	}

	RampTo(V1, num_steps, rampBack);

	if (rampBack > 0)
		SetNamePrefix(names[0]);

	cout << Exy.getPrefixedName() << " = " << to_string(Exy.Value()) << endl;
	UpdateEhv();
	emit sig_update_data();
}


void Voltages::RampTo(const std::vector<double>& V, unsigned nSteps, unsigned rampBack)
{
	QWriteLocker lock(&page_lock);

	double a, b;

	SetEndcaps(V[0], V[1], V[7], &a, &b);
	SetExy(0, V[2], V[3], &a, &b, V[4], V[5]);
	V6->SetValue(V[6]);

	UpdateAll(nSteps, rampBack);
}

void Voltages::UpdateAll(unsigned nSteps, unsigned rampBack)
{
	theApp->fpga->collectVoltageUpdates(true);

	for_each(voltage_parameters.begin(), voltage_parameters.end(), mem_fun(&AnalogOutParameter::UpdateOutput));

//   tNI.ForceUpdates();

	theApp->fpga->collectVoltageUpdates(false, nSteps, DwellTime, rampBack);
}

bool Voltages::RecalculateParameters()
{
	QWriteLocker lock(&page_lock);

	cout << "[Voltages::RecalculateParameters] entry" << endl;
//   cout << Exy.getPrefixedName() << " = " << to_string(Exy.Value()) << endl;

	bool Changed = FPGA_GUI::RecalculateParameters();

	if (NoUpdates.Value())
	{
		SetNamePrefix(SettingsName);
		cout << "[Voltages::RecalculateParameters] exit" << endl;
		return true;
	}

	UpdateInverseMatrices();

	double a, b;

	if (SettingsName.Value() != oldSettingsName)
	{
		RampSettings(oldSettingsName, SettingsName, RampSteps);
		oldSettingsName = SettingsName;
	}

	V6->SetValue(guiV6);

	SetEndcaps(ECratio, ECmean, ECtwist, &a, &b);

	if (ControlMode.Value() == "XY")
	{
		Exy.SetReadOnly(false);
		Ehv.SetReadOnly(true);


		SetExy(0, Exy.element(0, 0), Exy.element(0, 1), &a, &b, cmCE, cmRF);
		UpdateEhv();

		//Exy.UpdateGUI();
	}
	else
	{
		Exy.SetReadOnly(true);
		Ehv.SetReadOnly(false);

		SetExy(1, Ehv.element(0, 0), Ehv.element(0, 1), &a, &b, cmCE, cmRF);
		UpdateExy();
		//Ehv.UpdateGUI();
	}

	UpdateAll();

	//  cout << Exy.getPrefixedName() << " = " << to_string(Exy.Value()) << endl;
	cout << "[Voltages::RecalculateParameters] exit" << endl;

	return Changed;
}

void Voltages::UpdateEhv()
{
	QWriteLocker lock(&page_lock);

	Ehv.SetElement(0, 0, GetEh());
	Ehv.SetElement(0, 1, GetEv());
}

void Voltages::UpdateExy()
{
	QWriteLocker lock(&page_lock);

	Exy.SetElement(0, 0, GetEx());
	Exy.SetElement(0, 1, GetEy());
}

void Voltages::SetE(int i, double d)
{
	QWriteLocker lock(&page_lock);

	if (fabs(d) > 500)
		cout << "WARNING" << endl;

	cout << "E(" << i << ") = " << d << endl;



	switch (i)
	{
	case 0: SetEx(d); break;
	case 1: SetEy(d); break;
	case 2: SetECmean(d); break;
	case 3: SetECratio(d); break;
	case 4: SetEh(d); break;
	case 5: SetEv(d); break;
	}

	UpdateEhv();
	UpdateExy();
	UpdateAll();
}

double Voltages::GetE(int i)
{
	QWriteLocker lock(&page_lock);

	switch (i)
	{
	case 0: return GetEx();
	case 1: return GetEy();
	case 2: return GetECmean();
	case 3: return GetECratio();
	case 4: return GetEh();
	case 5: return GetEv();
	}

	throw runtime_error("[Voltages::GetE] bad index");
}

void Voltages::slot_on_action(QString s)
{
	std::string s2(s.toAscii());

	on_action(s2);
}
