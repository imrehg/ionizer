#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLCDNumber>
#include <QCheckBox>
#include <QProgressBar>
#include <QComboBox>

#include "DDS_Pulse_Widget.h"
#include "MatrixWidget.h"

#include "Widgets.h"
#include "ParameterGUI_Base.h"

using namespace std;

/*************************************************************************/
// void SetWidgetValue(W* w, const v& v) template specializations
// Write the value v to the GUI

template<> void SetWidgetValue(QLineEdit* w, const std::string& v)
{
	w->setText(v.c_str());
}

/*
   template<> void SetWidgetValue(QComboBox* w, const std::string& v)
   {
   int i = w->findText(v.c_str(), Qt::MatchExactly);

   if(i >= 0)
      w->setCurrentIndex(i);
   }
 */

template<> void SetWidgetValue(QSpinBox* w, const int& v)
{
	w->setValue(v);
}

template<> void SetWidgetValue(QSpinBox* w, const unsigned& v)
{
	w->setValue(v);
}

template<> void SetWidgetValue(QDoubleSpinBox* w, const double& v)
{
	w->setValue(v);
}

template<> void SetWidgetValue(QLCDNumber* w, const double& v)
{
	char buff[20] = "";

	snprintf(buff, 19, "%9.4f", v);
	w->display(buff);
}

template<> void SetWidgetValue(QCheckBox* w, const unsigned& v)
{
	w->setCheckState(v ? Qt::Checked : Qt::Unchecked);
}

template<> void SetWidgetValue(QProgressBar* w, const double& v)
{
	w->setValue((int)floor(v * 1000 + 0.5));
}

template<> void SetWidgetValue(QPushButton* w, const std::string& v)
{
	w->setText(v.c_str());
}

template<> void SetWidgetValue(DDS_Pulse_Widget* w, const dds_pulse_info& v)
{
	w->setPulseInfo(v);
}

template<> void SetWidgetValue(MatrixWidget* w, const matrix_t& v)
{
	w->setValue(v);
}


template<> void SetWidgetValue(TTL_Pulse_Widget* w, const ttl_pulse_info& v)
{
	w->setPulseInfo(v);
}

template<> void SetWidgetValue(QComboBox*, const std::string&)
{
	throw runtime_error("Error: SetWidgetValue(QComboBox*, const std::string&) should never be called");
}

template<> void SetWidgetReadOnly(QLineEdit* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(QSpinBox* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(QDoubleSpinBox* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(QComboBox* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(QCheckBox* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(QPushButton* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(DDS_Pulse_Widget* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(TTL_Pulse_Widget* w, bool b)
{
	w->setEnabled(!b);
}
template<> void SetWidgetReadOnly(MatrixWidget* w, bool b)
{
	w->setReadOnly(b);
}
template<> void SetWidgetReadOnly(QLCDNumber*, bool)
{
}
template<> void SetWidgetReadOnly(QProgressBar*, bool)
{
}


/*************************************************************************/
// std::string GetWidgetString(W* w) template specializations
// return the currently displayed string from the GUI

template<> std::string GetWidgetString(QLineEdit* w)
{
	return string(w->text().toAscii());
}

template<> std::string GetWidgetString(QComboBox* w)
{
	return string(w->currentText().toAscii());
}

template<> std::string GetWidgetString(QSpinBox* w)
{
	return to_string<int>(w->value());
}


template<> std::string GetWidgetString(QDoubleSpinBox* w)
{
	//there appears to be a bug if we use w->text().toStdString()
	return string(w->text().toAscii());
}

template<> std::string GetWidgetString(QLCDNumber* w)
{
	return to_string<double>(w->value());
}

template<> std::string GetWidgetString(QCheckBox* w)
{
	return to_string<bool>(w->checkState() == 2);
}

template<> std::string GetWidgetString(QProgressBar* w)
{
	return to_string<double>(w->value() * 0.001);
}

template<> std::string GetWidgetString(QPushButton* w)
{
	return string(w->text().toAscii());
}

template<> std::string GetWidgetString(DDS_Pulse_Widget* w)
{
	char buff[256];

	w->getPulseInfo().to_string(buff, 256);

	return string(buff);
}

template<> std::string GetWidgetString(TTL_Pulse_Widget* w)
{
	char buff[256];

	w->getPulseInfo().to_string(buff, 256);

	return string(buff);
}

template<> std::string GetWidgetString(MatrixWidget* w)
{
	return to_string<matrix_t>(w->getValue());
}


/*************************************************************************/
// void InitInputWidget(W* w, ParameterGUI_Base* r) template specializations
// initialize the widget, and make callback connections to ParameterGUI_Base* r

template<> void InitInputWidget(QLineEdit* w, ParameterGUI_Base* r)
{
	QObject::connect(w, SIGNAL(textEdited(const QString &)), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(QComboBox*, ParameterGUI_Base*)
{
}

template<> void InitInputWidget(QSpinBox* w, ParameterGUI_Base* r)
{
	w->setRange((int)(r->bottom), (int)(r->top));
	w->setSuffix(r->suffix.c_str());
	w->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);

	QObject::connect(w, SIGNAL(valueChanged(int)), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(QDoubleSpinBox* w, ParameterGUI_Base* r)
{
	w->setSingleStep(r->increment);
	w->setDecimals(r->precision);
	w->setRange(r->bottom, r->top);
	w->setSuffix(r->suffix.c_str());
	w->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);

	QObject::connect(w, SIGNAL(valueChanged(double)), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(QLCDNumber* w, ParameterGUI_Base*)
{
	QPalette pal;

	pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(40, 120, 40));
	pal.setColor(QPalette::WindowText, QColor(40, 120, 40));

	w->setPalette(pal);
	w->setSmallDecimalPoint(false);
	w->setNumDigits(8);

	w->setSegmentStyle(QLCDNumber::Flat);
}


template<> void InitInputWidget(QCheckBox* w, ParameterGUI_Base* r)
{
	QObject::connect(w, SIGNAL(stateChanged(int)), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(QProgressBar* w, ParameterGUI_Base*)
{
	w->setRange(0, 1000);
	w->setTextVisible(false);
}

template<> void InitInputWidget(QPushButton* w, ParameterGUI_Base* r)
{
	QObject::connect(w, SIGNAL(clicked(bool)), r, SLOT(GUI_button_press()), Qt::AutoConnection);
}

template<> void InitInputWidget(DDS_Pulse_Widget* w, ParameterGUI_Base* r)
{
	w->setName(r->get_fpga_name());

	QObject::connect(w, SIGNAL(valueChanged()), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(TTL_Pulse_Widget* w, ParameterGUI_Base* r)
{
	w->setName(r->get_fpga_name());
	QObject::connect(w, SIGNAL(valueChanged()), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}

template<> void InitInputWidget(MatrixWidget* w, ParameterGUI_Base* r)
{
//	w->setName(r->GetName());
	QObject::connect(w, SIGNAL(valueChanged()), r, SLOT(GUI_value_change()), Qt::AutoConnection);
}



template<> void selectScanWidget(QLineEdit*, const std::string&, bool)
{
}
template<> void selectScanWidget(QSpinBox*, const std::string&, bool)
{
}
template<> void selectScanWidget(QComboBox*, const std::string&, bool)
{
}
template<> void selectScanWidget(QCheckBox*, const std::string&, bool)
{
}
template<> void selectScanWidget(QPushButton*, const std::string&, bool)
{
}
template<> void selectScanWidget(QLCDNumber*, const std::string&, bool)
{
}
template<> void selectScanWidget(QProgressBar*, const std::string&, bool)
{
}
template<> void selectScanWidget(MatrixWidget*, const std::string&, bool)
{
}

// palScanning.setColor(QPalette::Base, QColor(150, 170, 255));

QPalette palScanning;
QPalette palInitial;

template<> void selectScanWidget(QDoubleSpinBox* w, const std::string&, bool bScan)
{
	if (bScan)
	{
		palInitial = w->palette();
		palScanning.setColor(QPalette::Base, QColor(150, 170, 255));   //light-blue
	}

	w->setPalette(bScan ? palScanning : palInitial);
}

template<> void selectScanWidget(DDS_Pulse_Widget* w, const std::string& scan_type, bool bScan)
{
	w->selectScanWidget(scan_type, bScan);
}

template<> void selectScanWidget(TTL_Pulse_Widget* w, const std::string& scan_type, bool bScan)
{
	w->selectScanWidget(scan_type, bScan);
}
