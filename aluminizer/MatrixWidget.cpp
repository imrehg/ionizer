#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QHeaderView>

#include "MatrixWidget.h"
#include "dds_pulse_info.h"

using namespace std;

MatrixWidget::MatrixWidget(QWidget* parent, const std::string&, unsigned nR, unsigned nC,
                           const std::string& sIni) :
	QTableWidget(nR, nC, parent),
	nR(nR),
	nC(nC),
	M(nR, nC),
	rLabels(nR),
	cLabels(nC),
	action_lock("Locked", this),
	action_unlock("Unlocked", this)
{
	if (nR > 1)
		for (unsigned i = 0; i < nR; i++)
		{
			std::string lbl;

			char sKey[64];
			snprintf(sKey, 64, "hr%d=", i);

			if (extract_val<string>(sIni, sKey, &lbl))
				rLabels[i] = lbl;
			else
				rLabels[i] = to_string<int>(i);
		}
	else
	{
	}

	for (unsigned j = 0; j < nC; j++)
	{
		std::string lbl;

		char sKey[64];
		snprintf(sKey, 64, "hc%d=", j);

		if (extract_val<string>(sIni, sKey, &lbl))
			cLabels[j] = lbl;
		else
			cLabels[j] = to_string<int>(j);
	}

	double range = 1e9;
	extract_val<double>(sIni, "range=", &range);

	for (unsigned i = 0; i < nR; i++)
	{
		for (unsigned j = 0; j < nC; j++)
		{
			setColumnWidth(j, 50);
			setItem(i, j,  new QTableWidgetItem());
			setCellWidget(i, j, new QDoubleSpinBox());
			input(i, j)->setDecimals(4);
			input(i, j)->setSingleStep(0.01);
			input(i, j)->setRange(-range, range);
			input(i, j)->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			input(i, j)->setButtonSymbols(QAbstractSpinBox::NoButtons);

			QObject::connect(input(i, j), SIGNAL(valueChanged(double)), this, SLOT(slot_valueChanged()), Qt::AutoConnection);
		}
	}

	action_lock.setCheckable(true);
	action_unlock.setCheckable(true);

	addAction(&action_lock);
	addAction(&action_unlock);
	setContextMenuPolicy(Qt::ActionsContextMenu);
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QObject::connect(&action_lock, SIGNAL(triggered(bool)), this, SLOT(slot_lock()), Qt::AutoConnection);
	QObject::connect(&action_unlock, SIGNAL(triggered(bool)), this, SLOT(slot_unlock()), Qt::AutoConnection);


	updateLabels();
	show();
}

MatrixWidget::~MatrixWidget()
{
}

void MatrixWidget::slot_itemActivated(QTableWidgetItem* twi)
{
	int r = twi->row();
	int c = twi->column();

	printf("r = %d  c = %d\r\n", r, c);
}

void MatrixWidget::slot_cellEntered(int r, int c)
{
	printf("cell entered: r = %d  c = %d\r\n", r, c);
}



QDoubleSpinBox* MatrixWidget::input(unsigned i, unsigned j)
{
	return dynamic_cast<QDoubleSpinBox*>(cellWidget(i, j));
}

void MatrixWidget::slot_valueChanged()
{
	emit valueChanged();
}

void MatrixWidget::setReadOnly(bool b)
{
	for (unsigned i = 0; i < nR; i++)
		for (unsigned j = 0; j < nC; j++)
			cellWidget(i, j)->setEnabled(!b);

	action_unlock.setChecked(!b);
	action_lock.setChecked(b);
}

void MatrixWidget::slot_lock()
{
	setReadOnly(true);
}

void MatrixWidget::slot_unlock()
{
	setReadOnly(false);
}

void MatrixWidget::updateLabels()
{
	for (unsigned j = 0; j < nC; j++)
	{
		QTableWidgetItem* twi = new QTableWidgetItem(cLabels[j].c_str());
		setHorizontalHeaderItem(j, twi);
	}

	horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	for (unsigned i = 0; i < nR; i++)
		setVerticalHeaderItem(i, new QTableWidgetItem(rLabels[i].c_str()));

	verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}


void MatrixWidget::setRowLabel(unsigned r, const std::string& s)
{
	rLabels.at(r) = s;
	updateLabels();
}

void MatrixWidget::setColLabel(unsigned c, const std::string& s)
{
	cLabels.at(c) = s;
	updateLabels();
}

void MatrixWidget::setValue(const matrix_t& m)
{
	for (unsigned i = 0; i < std::min<unsigned>(nR, m.nr); i++)
		for (unsigned j = 0; j < std::min<unsigned>(nC, m.nc); j++)
		{
			if (M.element(i, j) != m.element(i, j))
			{
				M.element(i, j) = m.element(i, j);
				input(i, j)->setValue(M.element(i, j));
			}

		}

}

double MatrixWidget::getValue(unsigned r, unsigned c)
{
	return input(r, c)->value();
}

const matrix_t& MatrixWidget::getValue()
{
	for (unsigned i = 0; i < nR; i++)
		for (unsigned j = 0; j < nC; j++)
			M.element(i, j) = input(i, j)->value();



	return M;
}


void CopyArray2D(const matrix_t& src, TNT::Array2D<double>* dst)
{
	for (int i = 0; i < dst->dim1(); i++)
		for (int j = 0; j < dst->dim2(); j++)
			(*dst)[i][j] = src.element(i, j);
}
