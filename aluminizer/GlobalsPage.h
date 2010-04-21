#pragma once

#include "ExperimentPage.h"

class Al3P1Page;
class Al3P0Page;
class TransitionPage;

class GlobalsPage : public ParamsPage
{
public:
GlobalsPage(const std::string& sPageName, ExperimentsSheet* pSheet);
virtual ~GlobalsPage();

virtual void PostCreateGUI();
void RememberWindowPos();

void SetIonXtal(const std::string);
std::string GetIonXtal();

unsigned NumAl();
unsigned NumMg();

protected:
virtual bool RecalculateParameters();



public:
GUI_unsigned FontSize;
GUI_unsigned WidgetSpacing;
GUI_unsigned NumColumns;
GUI_int HistogramSize;
GUI_int ReplotModulo;
GUI_int TimeSlice;

GUI_int PlotLineWidth;
GUI_bool ScatterScan;
GUI_bool DDS0Hz;
GUI_combo SavePlotType;

GUI_combo IonXtal;
GUI_bool recordCameraXY;

InputParameter<int>  xpos;
InputParameter<int>  ypos;

const std::string startup_IonXtal;
};

