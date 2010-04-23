#pragma once

class ParameterGUI_Base;



class QBaseSpinBox : public QSpinBox
{
public:
QBaseSpinBox(int base = 10, QWidget* parent = 0) :
	QSpinBox(parent), base_(base)
{
	// need to turn the validator off if you want to enter text manually
	//setValidator(0);
}

protected:
QString mapValueToText(int v)
{
	QString text;

	text.setNum(v, base_);
	return text;
};

int mapTextToValue(bool* ok)
{
	return text().toInt(ok, base_);
};

int base_;
};

class ColorPicker : public QPushButton
{
	Q_OBJECT

public:
    ColorPicker(QWidget* parent = 0);
	const QColor& value();

public slots:
	void setValue(int u);
	void rightClick();

signals:
	void valueChanged(unsigned);

protected:
	QColor clr;
};

template<class W, class V> void SetWidgetValue(W* w, const V& v);
template<class W> void SetWidgetReadOnly(W* w, bool b);
template<class W> std::string GetWidgetString(W* w);
template<class W> void InitInputWidget(W* input, ParameterGUI_Base* r);
template<class W> void selectScanWidget(W* input, const std::string& scan_type, bool bScan);
