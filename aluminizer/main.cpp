#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"

#include "InputParametersGUI.h"
#include "ExperimentsSheet.h"

double g_t0;
std::string g_t0s;

int main(int argc, char *argv[])
{
	g_t0 = CurrentTime_s();
	g_t0s = GetDateTimeString();

	AluminizerApp app(argc, argv);
	theApp = &app;

//	 app.InitFPGA();

	return app.exec();
}

