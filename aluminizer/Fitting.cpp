#include "plotlib/lm_lsqr.h"
#include "Numerics.h"
#include "Fitting.h"
#include "data_plot.h"
#include <sstream>

using namespace std;

namespace numerics
{

FitObject::FitObject(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, string output_dir) :
	m_xy(xy),
	m_x(m_xy->size()),
	m_y(m_xy->size()),
	m_yfit(m_xy->size()),
	base_params(10, 1),
	IsGoodFit(false),
	output_dir(output_dir),
	dx(1)
{
	if (m_xy->size() < MinDataPoints())
		return;

	std::sort(m_xy->begin(), m_xy->end());

	for (size_t i = 0; i < m_xy->size(); i++)
	{
		m_x[i] = (*m_xy)[i][x_column];
		m_y[i] = (*m_xy)[i][y_column]; //TODO: make this more robust
	}

	//bin for equal x-coordinates
	unsigned num_unique = 1;
	for (size_t i = 1; i < m_x.size(); i++)
		if (m_x[i] != m_x[i - 1])
			num_unique++;

	xb = std::vector<double>(num_unique, 0);
	yb = std::vector<double>(num_unique, 0);

	unsigned j = 0;
	unsigned nbin = 1;

	xb[0] = m_x[0];
	yb[0] = m_y[0];

	for (size_t i = 1; i < m_x.size(); i++)
	{
		if ( (m_x[i] != m_x[i - 1]) )
		{
			yb[j] /= nbin;
			j++;

			xb[j] = m_x[i];
			nbin = 0;
		}

		yb[j] += m_y[i];
		nbin++;
	}

	yb[j] /= nbin;

	dx = fabs(xb[0] - xb.back()) / xb.size();
}

FitObject::~FitObject()
{
	while (!Guesses.empty())
	{
		delete Guesses.back();
		Guesses.pop_back();
	}
}

void FitObject::fcn(const double* params, double* fvec)
{
	fitfunction(params, m_x, m_yfit);

	for (size_t i = 0; i < m_y.size(); i++)
		fvec[i] = m_y[i] - m_yfit[i];
}

bool FitObject::HasValidData()
{
	return base_params.size() > 0 && fit_params.size() > 0 && m_x.size() > MinDataPoints();
}

bool FitObject::DoFit()
{
	if ( m_x.size() <= MinDataPoints() )
		return false;

	ParamsGuess* BestGuess = 0;

	for (size_t i = 0; i < Guesses.size(); i++)
	{

		base_params = Guesses[i]->GuessBaseParams(this);
		fit_params = Guesses[i]->InitialFitParams(this);

		cout << endl;
		cout << "===== Initial parameters ======" << endl << endl;

		UpdateParams();

		PrintParams(&cout);

		cout << endl;
		cout << "===============================" << endl << endl;

		LM_LeastSquare lm_lsqr(static_cast<int>(m_x.size()), fit_params, this);

		lm_lsqr.Fit();

		fit_params = lm_lsqr.GetVariables();

		Guesses[i]->fit_params = fit_params;
		Guesses[i]->EuclideanNorm = lm_lsqr.GetEuclideanNorm();

		BestGuess = BetterGuess(Guesses[i], BestGuess);

		cout << "===== Final parameters ======" << endl << endl;

		UpdateParams();

		PrintParams(&cout);

		ofstream ff((output_dir + "fit.txt").c_str());
		PrintParams(&ff);


		cout << endl;
		cout << "===============================" << endl << endl;

	}

	if (BestGuess)
	{
		fit_params = BestGuess->fit_params;
		base_params = BestGuess->base_params;
		UpdateParams();
		CheckFit();
	}

	return HasValidData();
}

FitObject::ParamsGuess* FitObject::BetterGuess(FitObject::ParamsGuess* g1, FitObject::ParamsGuess* g2)
{
	if (!g1) return g2;
	if (!g2) return g1;

	return g1->EuclideanNorm < g2->EuclideanNorm ? g1 : g2;
}

void FitObject::PlotFit(data_plot* plot, double xOffset, double /*xScale*/)
{
	UpdateParams();

	unsigned nPlotPoints = 1000;

	xfit.resize(nPlotPoints);
	yfit.resize(xfit.size());

	double xMin = *min_element(m_x.begin(), m_x.end());
	double xMax = *max_element(m_x.begin(), m_x.end());
	double xRange = xMax - xMin;

	for (size_t i = 0; i < xfit.size(); i++)
		xfit[i] = xMin + i * xRange / (xfit.size() - 1);

	fitfunction(&(fit_params[0]), xfit, yfit);

	fitYmin = *min_element(yfit.begin(), yfit.end());
	fitYmax = *max_element(yfit.begin(), yfit.end());

	for (size_t i = 0; i < xfit.size(); i++)
		xfit[i] -= xOffset;

	ostringstream oss;
	PrintParams(&oss);
	string s = oss.str();

	for (size_t i = 0; i < s.length(); i++)
		if (s[i] == '\r' || s[i] == '\n')
			s[i] = ' ';

	plot->addCurveXY("FIT", xfit, yfit);
}

}

