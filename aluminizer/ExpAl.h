#pragma once

//#include "DDSsweeper.h"
#include "FrequencySource.h"

class Al3P1Page;
class Al3P0Page;
class MgPage;
class CoolingTransitionPage;
class RefCavityPage;
class ExpPrepareAl;
class MotionPage;
class Voltages;
class AgilisMotorsPage;

class ExpAl
{
public:
//	ExpAl(ExperimentsSheet* pSheet) {};

virtual ~ExpAl()
{
};

//	virtual Al3P1Page* GetAlTransition() = 0;

//	virtual CoolingTransitionPage* GetCoolingTransition() = 0;

static AgilisMotorsPage* pMirrorMotors;
static MotionPage* pMotion;
static MgPage* pMg;
static Al3P1Page* pAl3P1;
static Al3P0Page* pAl3P0;
};

