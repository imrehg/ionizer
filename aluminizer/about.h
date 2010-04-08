#pragma once

using namespace std;

#include "ExperimentPage.h"

class AboutPage : public ParamsPage
{
public:
AboutPage(ExperimentsSheet* pSheet);
virtual ~AboutPage();

GUI_string authors,buildDate;
GUI_string qtVersion, qwtVersion;
};
