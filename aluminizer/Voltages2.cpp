#include "Voltages2.h"
#include "FPGA_connection.h"
#include "ExperimentPage.h"
#include "GlobalsPage.h"

#include <QTableWidgetItem>
#include "MatrixWidget.h"

DACPage::DACPage(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	FPGA_GUI(sPageName, pSheet, page_id)
{
}

DACPage::~DACPage()
{
}

Voltages2::Voltages2(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	FPGA_GUI(sPageName, pSheet, page_id),
	pEMatrix(0),
	pXtallizeSettings(0),
	ReorderPeriod("Reorder period [s]",               &m_TxtParams, "10", &m_vParameters),
	ReorderDir("Direction", "MgAl\nAlMg\n", &m_TxtParams, "MgAl", &m_vParameters),
	FlipOrder("Flip order",              &m_TxtParams, "false", &m_vParameters),
	tLastReorder(0),
	flip_sign(1),
	cMgAl(0),
	cAlMg(0)
{
	ExperimentPage::pVoltages = this;

	for (int i = 0; i < 6; i++)
		g_scan_sources.push_back(new VScanSource(i, this));
}

Voltages2::~Voltages2()
{
}

void Voltages2::PostCreateGUI()
{
	pEMatrix = dynamic_cast<GUI_matrix*>(FindFPGAParameter("Eall"));
	pXtallizeSettings = dynamic_cast<GUI_unsigned*>(FindFPGAParameter("Xtallize settings"));

	if (pEMatrix)
	{
		pEMatrix->setToolTip("Voltages matrix -- each ion configuration has it's own settings.");

		pEMatrix->setInternalNamePrefix(ExperimentPage::pGlobals->GetIonXtal());

		MatrixWidget* pMW = pEMatrix->input_widget();

		QObject::connect( (QObject*)pMW->horizontalHeader(), SIGNAL( sectionClicked(int) ), this, SLOT( colSelection( int ) ) );
		pFPGA->SendParam(page_id, pEMatrix);
		pMW->selectColumn(0);
		colSelection(0);

		for (unsigned i = 0; i < (unsigned)(pMW->columnCount()); i++)
		{
			if (pMW->horizontalHeaderItem(i)->text().contains(QString("MgAl")))
				cMgAl = i;

			if (pMW->horizontalHeaderItem(i)->text().contains(QString("AlMg")))
				cAlMg = i;
		}
	}

	calcXtallizeSettings();


	FPGA_GUI::PostCreateGUI();
}

bool Voltages2::calcXtallizeSettings()
{
	if (pXtallizeSettings)
	{
		string s = ReorderDir.Value();

		if (strcmp(s.c_str(), "MgAl") == 0)
			return pXtallizeSettings->SetValue(cMgAl);
		else
			return pXtallizeSettings->SetValue(cAlMg);
	}

	return false;
}

bool Voltages2::RecalculateParameters()
{
	bool bChanged = FPGA_GUI::RecalculateParameters();

	bChanged |= calcXtallizeSettings();

	return bChanged;
}

int Voltages2::ReorderPeriodic(bool bForce)
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


		if (strcmp(s.c_str(), "MgAl") == 0)
		{
			pFPGA->rampVoltages(page_id, cMgAl, 1);
			dir = 1;
		}
		else
		{
			pFPGA->rampVoltages(page_id, cAlMg, 1);
			dir = -1;
		}
	}

	return dir;
}

void Voltages2::colSelection(int c)
{
	pFPGA->rampVoltages(page_id, c);
}


//convert E-index for voltage scans to row in matrix
unsigned Voltages2::Erow(unsigned i)
{
	switch (i)
	{
	case 0: return 0;
	case 1: return 1;
	case 2: return 4;
	case 3: return 5;
	case 4: return 2;
	case 5: return 3;
	}

	throw runtime_error("unknown E-field index");
}

double Voltages2::getE(unsigned i)
{
	unsigned r = Erow(i);

	return pEMatrix->element(r, 0);
}

void Voltages2::setE(unsigned i, double E)
{
	unsigned r = Erow(i);

	matrix_t m = pEMatrix->Value();

	m.element(r, 0) = E;

	pEMatrix->SetValue(m);
	pFPGA->SendParam(page_id, pEMatrix);
}

