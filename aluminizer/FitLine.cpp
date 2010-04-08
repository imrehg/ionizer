#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Fitting.h"
#include "Numerics.h"

using namespace std;
using namespace numerics;

FitLine::FitLine(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir)
{
	Guesses.push_back(new NormalGuess);
}

std::vector<double> FitLine::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitLine* pFL = dynamic_cast<FitLine*>(fit);

	base_params.resize(fit->BaseParamsSize());

	assert( pFL->m_y.size() >= 2 );
	//very crude initial guess for fit parameters
	base_params[Slope] = ( pFL->m_y.back() - pFL->m_y[0] ) / ( pFL->m_x.back() - pFL->m_x[0] ) ;
	base_params[Offset] = pFL->m_y[0];

	return base_params;
}

std::vector<double> FitLine::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	fit_params[Offset] = 0;

	return fit_params;
}

bool FitLine::CheckFit()
{
	IsGoodFit = true;

	fill(fit_params.begin(), fit_params.end(), 1);
	fit_params[Offset] = 0;


	return IsGoodFit;
}

void FitLine::UpdateParams()
{
	base_params[Slope] *= fit_params[Slope];
	base_params[Offset] += fit_params[Offset];

	fill(fit_params.begin(), fit_params.end(), 1);
	fit_params[Offset] = 0;
}


void FitLine::PrintParams(ostream* os)
{
	*os << "FitLine params" << endl;


	*os << setprecision(10);
	*os << setw(12) << "f0 = " << base_params[Offset] << endl;
	*os << setw(12) << "x0 = " << m_x[0] << endl;
	*os << setw(12) << "m = " << base_params[Slope]  << endl;
}


void FitLine::fitfunction(const double* params,
                          const std::vector<double>& x,
                          std::vector<double>& f_of_x)
{
	double l_f0 = base_params[Offset] + params[Offset];
	double l_m = base_params[Slope] * params[Slope];

	for (size_t i = 0; i < x.size(); i++)
		f_of_x[i] =  l_f0 + l_m * (x[i] - x[0]);
}

double FitLine::GetSlope() const
{
	return base_params[Slope];
}

double FitLine::GetOffset() const
{
	return base_params[Offset];
}

void FitLine::UpdateScanPage(ExpSCAN*)
{
	throw runtime_error("This isn't set up yet.");
}



