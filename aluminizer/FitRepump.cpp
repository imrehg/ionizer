#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Fitting.h"
#include "Numerics.h"

using namespace std;
using namespace numerics;

FitRepump::FitRepump(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir)
{
	Guesses.push_back(new NormalGuess);
}

std::vector<double> FitRepump::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRepump* pFL = dynamic_cast<FitRepump*>(fit);

	base_params.resize(fit->BaseParamsSize());

	assert( pFL->m_y.size() >= 2 );

	double max_x = *max_element(pFL->m_x.begin(), pFL->m_x.end());
	double max_y = *max_element(pFL->m_y.begin(), pFL->m_y.end());

	//very crude initial guess for fit parameters
	base_params[BkgRate] = max_y / max_x * 100;           //counts / 100 us
	base_params[BrightRate] = base_params[BkgRate] / 2;   //counts / 100 us
	base_params[tau] = max_x;

	return base_params;
}

std::vector<double> FitRepump::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	return fit_params;
}

bool FitRepump::CheckFit()
{
	IsGoodFit = true;

	return IsGoodFit;
}

void FitRepump::UpdateParams()
{
	base_params[BkgRate] *= fit_params[BkgRate];
	base_params[BrightRate] *= fit_params[BrightRate];
	base_params[tau] *= fit_params[tau];

	fill(fit_params.begin(), fit_params.end(), 1);
}


void FitRepump::PrintParams(ostream* os)
{
	*os << "FitRepump params" << endl;

	*os << setprecision(10);
	*os << setw(12) << "Bkg. rate = " << base_params[BkgRate] << " per 100 us" << endl;
	*os << setw(12) << "Bright rate = " << base_params[BrightRate] << " per 100 us" << endl;
	*os << setw(12) << "tau = " << base_params[tau] << " us" << endl;
}

// y(x) = BkgRate * x + BrightRate * ( (x + tau * exp(-x/tau) ) - tau )
void FitRepump::fitfunction(const double* params,
                            const std::vector<double>& x,
                            std::vector<double>& f_of_x)
{
	double l_BkgSlope = base_params[BkgRate] * params[BkgRate] / 100.0;
	double l_BrightRate = base_params[BrightRate] * params[BrightRate] / 100.0;
	double l_tau = base_params[tau] * params[tau];

	l_tau = max<double>(l_tau, 1);

	for (size_t i = 0; i < x.size(); i++)
	{
		f_of_x[i] =  l_BkgSlope * x[i];

		double epsilon = -1 * x[i] / l_tau;

		if (fabs(epsilon) < 1e-3)
			//use power-series expansion for better numerical accuracy
			f_of_x[i] += (l_BrightRate) * ((1 / 2.) * pow(epsilon, 2) + (1 / 6.) * pow(epsilon, 3) + (1 / 24.) * pow(epsilon, 4));
		else
			f_of_x[i] += (l_BrightRate) * ( x[i] + l_tau * (exp(-1 * x[i] / l_tau) - 1) );
	}
}

double FitRepump::GetBkgRate() const
{
	return base_params[BkgRate];
}

double FitRepump::GetBrightRate() const
{
	return base_params[BrightRate];
}

double FitRepump::GetTau() const
{
	return base_params[tau];
}

void FitRepump::UpdateScanPage(ExpSCAN*)
{
//	throw runtime_error("This isn't set up yet.");
}



