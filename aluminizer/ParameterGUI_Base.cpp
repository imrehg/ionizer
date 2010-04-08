#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ParameterGUI_Base.h"
#include "Widgets.h"

#include "dds_pulse_info.h"

using namespace std;

ParameterGUI_Base::ParameterGUI_Base(std::vector<ParameterGUI_Base*> * pv,
                                     unsigned flags) :
	Parameter_Base(""),
	label(0),
	parent(0),
	gui_revision(-2),
	bottom(-2e9),
	top(2e9),
	precision(3),
	increment(0.01),
	input_width(0),
	label_width(0),
	new_row(false),
	bLinked(false),
	flags(flags),
	bInit(true),
	fpga_id(-1)
{
	if (pv)
		pv->push_back(this);

	palBSB2.setColor(QPalette::WindowText, QColor(100, 100, 200));
	palBSB1.setColor(QPalette::WindowText, QColor(50, 50, 200));
	palRSB1.setColor(QPalette::WindowText, QColor(200, 50, 50));
	palRSB2.setColor(QPalette::WindowText, QColor(200, 100, 100));
	palCarrier.setColor(QPalette::WindowText, QColor(50, 170, 50));

	QObject::connect(this, SIGNAL(sig_UpdateGUI()), this, SLOT(UpdateGUI()), Qt::QueuedConnection);
}

ParameterGUI_Base::ParameterGUI_Base(std::vector<ParameterGUI_Base*> * pv,
                                     const std::string& sIni) :
	Parameter_Base(""),
	label(0),
	parent(0),
	gui_revision(-2),
	bottom(-2e9),
	top(2e9),
	precision(3),
	increment(0.01),
	input_width(0),
	label_width(0),
	new_row(false),
	bLinked(false),
	flags(0),
	bInit(true),
	fpga_id(-1)
{
	extract_val<double>(sIni, "min=", &bottom);
	extract_val<double>(sIni, "max=", &top);
	extract_val<unsigned>(sIni, "flags=", &flags);

	flags = flags & (~pulse_info::flag_disabled);

	if (pv)
		pv->push_back(this);

	palBSB2.setColor(QPalette::WindowText, QColor(100, 100, 200));
	palBSB1.setColor(QPalette::WindowText, QColor(50, 50, 200));
	palRSB1.setColor(QPalette::WindowText, QColor(200, 50, 50));
	palRSB2.setColor(QPalette::WindowText, QColor(200, 100, 100));
	palCarrier.setColor(QPalette::WindowText, QColor(50, 170, 50));

	QObject::connect(this, SIGNAL(sig_UpdateGUI()), this, SLOT(UpdateGUI()), Qt::QueuedConnection);
}

void ParameterGUI_Base::selectScanTarget(const std::string&, bool)
{
}


void ParameterGUI_Base::PostUpdateGUI()
{
	emit sig_UpdateGUI();
}

void ParameterGUI_Base::setLinked(bool bLinked)
{
	this->bLinked = bLinked;

	if (bLinked)
	{
		QColor bkg(Qt::lightGray);
		palBSB2.setColor(QPalette::Window, bkg);
		palBSB1.setColor(QPalette::Window, bkg);
		palRSB1.setColor(QPalette::Window, bkg);
		palRSB2.setColor(QPalette::Window, bkg);
		palCarrier.setColor(QPalette::Window, bkg);
		palOther.setColor(QPalette::Window, bkg);
	}

}

QWidget* ParameterGUI_Base::MakeLabelControl(QWidget* parent, unsigned)
{
	this->parent = parent;

	if (HasLabel())
	{
		label = new QLabel(parent);
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		label->setAutoFillBackground(true);

		UpdateGUI_Label();
	}

	return label;
}

void ParameterGUI_Base::GUI_button_press()
{
	emit GUI_button_was_pressed(this);
}

void ParameterGUI_Base::GUI_value_change()
{
	if (flags & RP_FLAG_READ_ONLY)
		return;

	gui_revision++;

	UpdateFromGUI();

	emit GUI_user_value_change(this);
}

void ParameterGUI_Base::Show(bool bShow)
{
	setFlag(RP_FLAG_HIDDEN, !bShow);
}

void ParameterGUI_Base::setFlag(unsigned f, bool bOn)
{
	if (bOn)
		flags |= f;
	else
		flags &= ~f;
}


const std::string& ParameterGUI_Base::GetName() const
{
	//return get_display_label();
	return IGetName();
}

bool ParameterGUI_Base::SetName(const std::string& name)
{
	gui_revision = -2;
	return ISetName(name);
}

void ParameterGUI_Base::setInputPalette(const QPalette& p)
{
	if (input_widget())
		input_widget()->setPalette(p);
}


void ParameterGUI_Base::UpdateGUI()
{
	if (label)
		UpdateGUI_Label();

	if (input_widget())
		UpdateGUI_Value();

	gui_revision = revision;
}

QPalette* ParameterGUI_Base::GetLabelPalette(const std::string& s)
{
	if (s.find("bsb") != string::npos || s.find("BSB") != string::npos)
	{
		if (s.find("COM") != string::npos)
			return &palBSB1;
		else
			return &palBSB2;
	}


	if (s.find("rsb") != string::npos || s.find("RSB") != string::npos)
	{
		if (s.find("COM") != string::npos)
			return &palRSB1;
		else
			return &palRSB2;
	}

	if (s.find("carrier") != string::npos)
		return &palCarrier;

	return &palOther;
}


void ParameterGUI_Base::UpdateGUI_Label()
{
	if (label)
	{
		if (flags & RP_FLAG_HIDDEN)
			label->hide();
		else
			label->show();

		QPalette* pal = GetLabelPalette(get_display_label().c_str());

		if (pal)
			label->setPalette(*pal);

		label->setText(get_display_label().c_str());

		if (ttip.length())
			label->setToolTip(ttip.c_str());
	}

	if (input_widget() && ttip.length())
	{
		input_widget()->setToolTip(ttip.c_str());
		ttip = "";
	}
}

bool ParameterGUI_Base::UpdateFromGUI()
{
	bool bResult = false;

	if (input_widget())
		if (gui_revision > revision)
		{
			bResult = SetValueFromString( GetGUIString() );
			gui_revision = revision;
		}

	return false;
}

bool ParameterGUI_Base::OwnsWindow(QWidget* p)
{
	return p && ((p == input_widget()) || (p == label));
}

QPoint ParameterGUI_Base::GetInputSize()
{
	return QPoint(200, 30);
}

QPoint ParameterGUI_Base::GetLabelSize()
{
	return IsPacked() ? QPoint(100, 30) : QPoint(200, 30);
}
