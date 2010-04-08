#pragma once

#include "DDS_Pulse_Widget.h"
#include "MatrixWidget.h"

#include "ParameterGUI_Base.h"
#include "Widgets.h"


std::string extractName(const std::string& sIni);

/*

   InputParameterGUI are parameters which have a GUI front-end, and a database backend.
   The data itself is a primitive data type.


 */


template <class W, class T, bool bHasLabel = true, bool bLabelIsValue = false> class InputParameterGUI : public InputParameter<T>, public ParameterGUI_Base
{
public:
InputParameterGUI(const std::string& sName,
                  InputParameters* pIPs,
                  const std::string& sDefault = "",
                  std::vector<ParameterGUI_Base*> * pv = 0,
                  unsigned flags = 0) :
	Parameter_Base(sName),
	InputParameter<T>(sName, pIPs, sDefault),
	ParameterGUI_Base(pv, flags),
	input(0)
{
}

InputParameterGUI(const std::string& sIni,
                  InputParameters* pIPs,
                  std::vector<ParameterGUI_Base*> * pv = 0) :
	Parameter_Base(extractName(sIni)),
	InputParameter<T>(extractName(sIni), pIPs, extractDefault(sIni)),
	ParameterGUI_Base(pv, sIni),
	input(0)
{
}

virtual ~InputParameterGUI()
{
}

virtual void initInputWidget()
{
	if (input)
		InitInputWidget(input, this);
}

std::string extractDefault(const std::string& sIni)
{
	T t;

	if (extract_val<T>(sIni, "value=", &t))
		return to_string<T>(t);
	else
		return "";
}

virtual W* CreateInputWidget(QWidget* parent)
{
	return new W(parent);
}

virtual QWidget* MakeInputControl(QWidget* parent, unsigned)
{
	input = CreateInputWidget(parent);
	SetReadOnly(IsReadOnly());
	InitInputWidget(input, this);
	PostCreateInputWidget();

	return input;
}

virtual void PostCreateInputWidget()
{
}

virtual void selectScanTarget(const std::string& scan_type, bool bScan)
{
	if (input)
		selectScanWidget(input, scan_type, bScan);
}

virtual std::string getValueString()
{
	return InputParameter<T>::m_pIPs->GetValue(Parameter_Base::IGetName());
}

virtual std::string GetGUIString()
{
	if (input)
		return GetWidgetString<W>(input);
	else
		return InputParameter<T>::m_pIPs->GetValue(Parameter_Base::IGetName());
}

virtual bool IsInitialized()
{
	return InputParameter<T>::IsInitialized();
}

virtual bool SetValueFromString(const std::string& s)
{
	return SetValue(from_string<T>(s));
}

virtual void UpdateGUI_Value()
{
	if (input)
	{
		if (flags & RP_FLAG_HIDDEN)
			input->hide();
		else
			input->show();
	}

	T val = InputParameter<T>::Value();

	if (input)
	{
		if (gui_revision < revision)
		{
			gui_revision = revision;
			T current_val = from_string<T>(GetGUIString());

			if (val != current_val)
			{
				if (bLabelIsValue)
					SetWidgetValue<W, T>(input, from_string<T>(GetName()));
				else
					SetWidgetValue<W, T>(input, val);
			}
		}

		SetWidgetReadOnly<W>(input, (flags & RP_FLAG_READ_ONLY));
	}
}

virtual W* input_widget() const
{
	return input;
}

virtual bool HasLabel() const
{
	return bHasLabel;
}

bool LinkTo(InputParameterGUI<W, T>* pIP)
{
	bool b = InputParameter<T>::LinkTo(pIP);

	if (b)
	{
		gui_revision = -2;
		setPrecision(pIP->precision);
		setSuffix(pIP->suffix);
		setLinked(true);
	}

	return b;
}

bool LinkTo(InputParameters* pIPs, const std::string& sName)
{
	bool b = InputParameter<T>::LinkTo(pIPs, sName);

	if (b)
	{
		gui_revision = -2;
		setLinked(true);
	}

	return b;
}

void SetReadOnly(bool bReadOnly)
{
	if (bReadOnly)
		flags = (flags | RP_FLAG_READ_ONLY);
	else
		flags = (flags & ~RP_FLAG_READ_ONLY);
}


W* input;
};

typedef InputParameterGUI<QLineEdit,   std::string, true>         GUI_string;

typedef InputParameterGUI<QBaseSpinBox,        int, true>         GUI_int_base;
typedef InputParameterGUI<QSpinBox,            int, true>         GUI_int;
typedef InputParameterGUI<QSpinBox,       unsigned, true>         GUI_unsigned;
typedef InputParameterGUI<QSpinBox,            int, false>        GUI_int_no_label;

typedef InputParameterGUI<QDoubleSpinBox,   double, true>         GUI_double;
typedef InputParameterGUI<QDoubleSpinBox,   double, false>        GUI_double_no_label;

typedef InputParameterGUI<QLCDNumber,     double, true>        GUI_doubleLCD;
typedef InputParameterGUI<QCheckBox,      unsigned, true>         GUI_bool;
typedef InputParameterGUI<QProgressBar,     double, true>         GUI_progress;
typedef InputParameterGUI<QPushButton, std::string, false, true>  GUI_textButton;

class GUI_matrix : public InputParameterGUI<MatrixWidget, matrix_t, false>
{
public:
GUI_matrix(const std::string& sIni,
           InputParameters* pIPs,
           unsigned nR, unsigned nC,
           std::vector<ParameterGUI_Base*> * pv = 0);

GUI_matrix(const std::string& sIni,
           InputParameters* pIPs,
           std::vector<ParameterGUI_Base*> * pv = 0);

virtual ~GUI_matrix()
{
}

virtual int min_height();
virtual int getNumGUIRows() const
{
	return 1 + nR;
}
virtual int getNumGUIColumns() const
{
	return std::max<int>(nC, 2);
}

virtual unsigned get_binary_length();
virtual void insert_binary(char* p);

virtual MatrixWidget* CreateInputWidget(QWidget* parent)
{
	return new MatrixWidget(parent, GetName(), nR, nC, sIni);
}

double element(unsigned r, unsigned c)
{
	matrix_t m = Value();

	if (c >= m.nc || r >= m.nr)
		return 0;
	else
		return m.element(r, c);
}

bool SetElement(unsigned r, unsigned c, double v)
{
	matrix_t m = Value();

	m.element(r, c) = v;

	return SetValue(m);
}

void setRowLabel(unsigned r, const std::string& s)
{
	if (input)
		input->setRowLabel(r, s);
}

void setColLabel(unsigned c, const std::string& s)
{
	if (input)
		input->setColLabel(c, s);
}

protected:
std::string sIni;
unsigned nR, nC;
};


class GUI_ttl : public InputParameterGUI<TTL_Pulse_Widget, ttl_pulse_info>
{
public:
GUI_ttl(const std::string& sIni,
        InputParameters* pIPs,
        std::vector<ParameterGUI_Base*> * pv = 0) :
	Parameter_Base(extractName(sIni)),
	InputParameterGUI<TTL_Pulse_Widget, ttl_pulse_info>(extractName(sIni), pIPs, extractDefault(sIni), pv, extract_flags(sIni) | RP_FLAG_NOPACK)
{
}

bool isEnabled()
{
	if (input)
		return input->bEnabled;
	else
		return false;
}

void setTime(double t)
{
	ttl_pulse_info i = Value();

	i.t = t;
	SetValue(i);
}

double getTime()
{
	ttl_pulse_info i = Value();

	return i.t;
}

protected:

std::string extractDefault(const std::string& sIni)
{
	ttl_pulse_info i;

	sscanf(sIni.c_str(), "ttl=%u t=%lf", &(i.ttl), &(i.t));

	return to_string(i);
}
};

class GUI_dds : public InputParameterGUI<DDS_Pulse_Widget, dds_pulse_info>
{
public:
GUI_dds(const std::string& sIni,
        InputParameters* pIPs,
        std::vector<ParameterGUI_Base*> * pv = 0) :
	Parameter_Base(extractName(sIni)),
	InputParameterGUI<DDS_Pulse_Widget, dds_pulse_info>(extractName(sIni), pIPs, extractDefault(sIni), pv, extract_flags(sIni) | RP_FLAG_NOPACK),
	bHas_fOff(extractHas_fOff(sIni)),
	bHas_SB(extractDefaultPulse(sIni).hasSB()),
	freq_unit(extractFreqUnit(sIni))
{
}

virtual DDS_Pulse_Widget* CreateInputWidget(QWidget* parent)
{
	return new DDS_Pulse_Widget(parent, freq_unit);
}

std::string extractDefault(const std::string& sIni)
{
	dds_pulse_info i;

	sscanf(sIni.c_str(), "dds=%u t=%lf fOn=%lf fOff=%lf sb=%d", &(i.iDDS), &(i.t), &(i.fOn), &(i.fOff), &(i.sb));

	return to_string(i);
}

dds_pulse_info extractDefaultPulse(const std::string& sIni)
{
	dds_pulse_info i;

	sscanf(sIni.c_str(), "dds=%u t=%lf fOn=%lf fOff=%lf sb=%d", &(i.iDDS), &(i.t), &(i.fOn), &(i.fOff), &(i.sb));

	return i;
}

bool extractHas_fOff(const std::string& sIni)
{
	dds_pulse_info i;

	sscanf(sIni.c_str(), "dds=%u t=%lf fOn=%lf fOff=%lf", &(i.iDDS), &(i.t), &(i.fOn), &(i.fOff));

	return i.fOff != 0;
}

double extractFreqUnit(const std::string& sIni)
{
	double u = 1e6;
	size_t loc = sIni.find("unit=");

	if (loc != string::npos)
		sscanf(sIni.substr(loc).c_str(), "unit=%lf", &u);

	return u;
}

void setTime(double t)
{
	dds_pulse_info i = Value();

	i.t = t;
	SetValue(i);
}

double getTime()
{
	dds_pulse_info i = Value();

	return i.t;
}

void setFreq(double f)
{
	dds_pulse_info i = Value();

	i.fOn = f;

	//	cout << "set: " << to_string<dds_pulse_info>(i) << endl;
	SetValue(i);

	//	cout << "get: " << to_string<dds_pulse_info>(Value())  << endl;
}

double getFreq()
{
	dds_pulse_info i = Value();

	return i.fOn;
}

void setFreqOff(double f)
{
	dds_pulse_info i = Value();

	i.fOff = f;
	SetValue(i);
}

virtual void PostCreateInputWidget()
{
	if (!bHas_fOff)
		input->disable_fOff();
}


protected:
bool bHas_fOff, bHas_SB;
double freq_unit;
};

class GUI_ZS : public GUI_double
{
public:
GUI_ZS(const std::string& sName,
       InputParameters* pIPs,
       double min, double max,
       std::vector<ParameterGUI_Base*> * pv = 0,
       unsigned flags = 0) :
	Parameter_Base(sName),
	GUI_double(sName, pIPs, to_string<double>(min), pv, flags)
{
	setRange(min, max);
}

virtual void PostCreateInputWidget()
{
	input->setSingleStep(1);

	if (IsHalfInteger(bottom))
		input->setDecimals(1);
	else
		input->setDecimals(0);
}

static bool IsHalfInteger(double d)
{
	return (floor(d) != d) && (floor(2 * d) == 2 * d);
}
};

class GUI_pol : public GUI_int
{
public:
GUI_pol(const std::string& sName,
        InputParameters* pIPs,
        const std::string& sDefault = "",
        std::vector<ParameterGUI_Base*> * pv = 0,
        unsigned flags = 0) :
	Parameter_Base(sName),
	GUI_int(sName, pIPs, sDefault, pv, flags)
{
	bottom = -1;
	top = 1;
}
/*
   void setRange(int b, int t)
   {
      GUI_int::setRange(b,t);
   //	if(input)
   //		input->setRange(b,t);
   }
 */
};

//typedef InputParameterGUI<QComboBox, std::string> GUI_combo;

class GUI_combo : public InputParameterGUI<QComboBox, std::string>
{
public:
GUI_combo(const std::string& sName,
          const std::string& sChoices,
          InputParameters* pIPs,
          const std::string& sDefault = "",
          std::vector<ParameterGUI_Base*> * pv = 0,
          unsigned flags = 0) :
	Parameter_Base(sName),
	InputParameterGUI<QComboBox, std::string>(sName, pIPs, sDefault, pv, flags),
	lockChangeChoices(QReadWriteLock::Recursive),
	bSyncChoicesGUI(true)
{
	AddChoices(sChoices);
}

virtual QWidget* MakeInputControl(QWidget* parent, unsigned id);
virtual void UpdateGUI_Value();

void AddChoices(const std::string& sChoices);
bool AddChoice(const std::string& sChoices);
bool RemoveChoice(const std::string& sChoice);
void clearChoices();
unsigned numChoices() const
{
	return choices.size();
}

protected:
std::vector<std::string> choices;
QReadWriteLock lockChangeChoices;
bool bSyncChoicesGUI;     //true if choices was updated and the GUI needs to sync
};

