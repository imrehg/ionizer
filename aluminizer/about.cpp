#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "about.h"
#include <qwt_global.h>

AboutPage::AboutPage(ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, "About"),
	authors("Authors:", &m_TxtParams, "", &m_vParameters, RP_FLAG_READ_ONLY | RP_FLAG_NOPACK),
	buildDate("Build date:",  &m_TxtParams, "", &m_vParameters, RP_FLAG_READ_ONLY | RP_FLAG_NOPACK),
	qtVersion("Qt version:", &m_TxtParams, QT_VERSION_STR, &m_vParameters, RP_FLAG_READ_ONLY | RP_FLAG_NOPACK),
	qwtVersion("Qwt version:", &m_TxtParams, QWT_VERSION_STR, &m_vParameters, RP_FLAG_READ_ONLY | RP_FLAG_NOPACK)

{
	authors.SetValue("Till Rosenband, David Leibrandt, Michael Thorpe, Piet O. Schmidt, David B. Hume, Chin-wen Chou");

	char buff[128];
	snprintf(buff, 128, "%s %s", __DATE__, __TIME__);
	buildDate.SetValue(buff);
}

AboutPage::~AboutPage()
{
}
