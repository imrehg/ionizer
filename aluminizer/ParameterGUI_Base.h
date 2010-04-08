#pragma once

#include "InputParameters.h"
#include "messaging.h"

class ParameterGUI_Base : public QObject, virtual public Parameter_Base
{
Q_OBJECT

public:
ParameterGUI_Base(std::vector<ParameterGUI_Base*> * pv = 0, unsigned flags = 0);
ParameterGUI_Base(std::vector<ParameterGUI_Base*> * pv, const std::string& sIni);
virtual ~ParameterGUI_Base()
{
}

void Show(bool);

virtual void PostUpdateGUI();

//indicate in the gui that this parameter is being scanned
virtual void   selectScanTarget(const std::string& scan_type, bool bScan);

void setToolTip(const std::string& s)
{
	ttip = s;
}

//! param ID on FPGA so it doesn't haver to be looked up by name
void set_fpga_id(int id)
{
	fpga_id = id;
}
int get_fpga_id()
{
	return fpga_id;
}

virtual unsigned get_binary_length()
{
	return 0;
}
virtual void insert_binary(char*)
{
}

virtual int getNumGUIRows() const
{
	return 1;
}
virtual int getNumGUIColumns() const
{
	return 1;
}

virtual int min_height()
{
	return 20;
}

virtual bool canHide() const
{
	return flags & RP_FLAG_CAN_HIDE;
}

virtual bool HasLabel() const
{
	return true;
}

virtual QWidget* MakeLabelControl(QWidget* parent, unsigned id);
virtual QWidget* MakeInputControl(QWidget* parent, unsigned id) = 0;
virtual QWidget* input_widget() const = 0;

virtual void SetReadOnly(bool bReadOnly) = 0;

void setSuffix(const std::string& s)
{
	suffix = s; initInputWidget();
}

virtual void forceNewRow()
{
	new_row = true;
}

virtual void setLabelWidth(unsigned w)
{
	label_width = w;
}
virtual void setInputWidth(unsigned w)
{
	input_width = w;
}

virtual std::string getValueString() = 0;

virtual void setRange(double b, double t)
{
	if (bottom != b || top != t)
	{
		bottom = b;
		top = t;
		initInputWidget();
	}
}

virtual void setPrecision(int p)
{
	precision = p; initInputWidget();
}
virtual void setIncrement(double d)
{
	increment = d; initInputWidget();
}
virtual void setInputPalette(const QPalette& p);

bool OwnsWindow(QWidget*);
bool IsPacked()
{
	return !(flags & RP_FLAG_NOPACK);
}
bool IsReadOnly()
{
	return flags & RP_FLAG_READ_ONLY;
}

virtual const std::string& GetName() const;

virtual bool SetName(const std::string& name);

void setInternalNamePrefix(const std::string& s)
{
	std::string n = get_display_label();

	set_display_label(n);

	SetName(s + " " + n);
}


//return string currently shown in GUI
virtual std::string GetGUIString() = 0;

virtual bool SetValueFromString(const std::string& s) = 0;

friend class TxtParametersGUI;

void setLinked(bool bLinked);
bool isLinked() const
{
	return bLinked;
}

void addFlag(unsigned f)
{
	flags |= f;
}
void removeFlag(unsigned f)
{
	flags &= ~f;
}

void setFlag(unsigned f, bool bOn);
void setFlags(unsigned f)
{
	flags = f;
}
unsigned getFlags() const
{
	return flags;
}

void set_gui_revision(int i)
{
	gui_revision = i;
}

protected slots:
void UpdateGUI();

signals:
void sig_UpdateGUI();

protected:

virtual void UpdateGUI_Label();

//update the value displayed in the GUI, from the current internal paramater value
virtual void UpdateGUI_Value() = 0;

virtual QPoint GetInputSize();
virtual QPoint GetLabelSize();

QWidget* label_widget() const
{
	return label;
}

virtual bool UpdateFromGUI();

virtual void initInputWidget() = 0;

public slots:
void GUI_value_change();
void GUI_button_press();

signals:
void GUI_user_value_change(ParameterGUI_Base*);
void GUI_button_was_pressed(ParameterGUI_Base*);

protected:
QPalette* GetLabelPalette(const std::string& s);

QPalette palBSB1, palBSB2, palRSB1, palRSB2;
QPalette palCarrier;
QPalette palOther;

QLabel*  label;
QWidget*  parent;

int gui_revision;

public:
std::string suffix;
double bottom;
double top;
int precision;
double increment;
unsigned input_width;
unsigned label_width;


bool new_row;

std::string tool_tip;

bool bLinked;

unsigned flags;

std::string ttip;
bool bInit;
int fpga_id;
};
