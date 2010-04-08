#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Fitting.h"
#include "Numerics.h"
#include "ExpSCAN.h"

using namespace std;
using namespace numerics;


FitRamseyFreq::FitRamseyFreq(std::vector< std::vector<double> >* xy,
                             size_t x_column, size_t y_column,
                             double MinContrast, double MinAmplitude,
                             int direction, double omegaR, double RamseyT, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir),
	MinContrast(MinContrast),
	MinAmplitude(MinAmplitude),
	RamseyT(RamseyT),
	direction(direction),
	omegaR(omegaR)
{
	assert(m_x.size() == m_y.size());
	Guesses.push_back(new NormalGuess);
	Guesses.push_back(new PeakGuess);
}

std::vector<double> FitRamseyFreq::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRamseyFreq* pFRF = dynamic_cast<FitRamseyFreq*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters
	base_params[T] = pFRF->RamseyT;
	base_params[OmegaRange] = omega(pFRF->m_x.back() - pFRF->m_x.front());
	base_params[OmegaR] = pFRF->omegaR;

	double lowest = *min_element(pFRF->m_y.begin(), pFRF->m_y.end());
	double highest = *max_element(pFRF->m_y.begin(), pFRF->m_y.end());
	double mean = (lowest + highest) / 2;

	//find the mean crossing closest to the center that goes in the right direction
	int center = static_cast<int>(pFRF->m_y.size()) / 2;
	int left = center;

	while (--left >= 0 )
		if ( ( pFRF->m_y[left] > mean ) != ( pFRF->m_y[left + 1] > mean ) )
			if ((pFRF->m_y[left] - mean) * (pFRF->direction) < 0)
				break;

	int right = center;

	while (++right < static_cast<int>(pFRF->m_y.size()) )
		if ( ( pFRF->m_y[right] > mean ) != ( pFRF->m_y[right - 1] > mean ) )
			if ((pFRF->m_y[right] - mean) * (pFRF->direction) < 0)
				break;

	if ((pFRF->m_y[center] - mean) * (pFRF->direction) < 0)
	{
		if ((right - center) < (center - left))
		{
			left = right;
			while (++right < static_cast<int>(pFRF->m_y.size()) )
				if ( ( pFRF->m_y[right] > mean ) != ( pFRF->m_y[center] > mean ) )
					break;
		}
		else
		{
			right = left;
			while (--left >= 0 )
				if ( ( pFRF->m_y[left] > mean ) != ( pFRF->m_y[center] > mean ) )
					break;
		}
	}

	center = (right + left) / 2;
	base_params[Omega0] = omega(pFRF->m_x[center]);

	if (pFRF->direction < 0)
	{
		base_params[Amplitude] = lowest - highest;
		base_params[Offset] = highest;
	}
	else
	{
		base_params[Amplitude] = highest - lowest;
		base_params[Offset] = lowest;
	}

	return base_params;
}

std::vector<double> FitRamseyFreq::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	fit_params[Omega0] = 0;

	return fit_params;
}

std::vector<double> FitRamseyFreq::PeakGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRamseyFreq* pFRF = dynamic_cast<FitRamseyFreq*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters
	base_params[T] = pFRF->RamseyT;
	base_params[OmegaRange] = omega(pFRF->m_x.back() - pFRF->m_x.front());
	base_params[OmegaR] = pFRF->omegaR;

	double lowest = *min_element(pFRF->m_y.begin(), pFRF->m_y.end());
	double highest = *max_element(pFRF->m_y.begin(), pFRF->m_y.end());
	base_params[Offset] = ( pFRF->m_y.back() + pFRF->m_y.front() ) / 2 ;

	size_t peak = 0;

	if ( pFRF->direction < 0 )
	{
		peak = find(pFRF->m_y.begin(), pFRF->m_y.end(), lowest) - pFRF->m_y.begin();
		base_params[Offset] = highest;
	}
	else
	{
		peak = find(pFRF->m_y.begin(), pFRF->m_y.end(), highest) - pFRF->m_y.begin();
		base_params[Offset] = lowest;
	}

	base_params[Amplitude] = pFRF->m_y[peak] - base_params[Offset];
	base_params[Omega0] = omega(pFRF->m_x[peak]);

	return base_params;
}

std::vector<double> FitRamseyFreq::PeakGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	fit_params[Omega0] = 0;

	return fit_params;
}

bool FitRamseyFreq::CheckFit()
{
	IsGoodFit = true;

	//check fitted amplitude
	cout << "fitted amplitude = " << fabs(base_params[Amplitude]) << endl;
	IsGoodFit &=  fabs(base_params[Amplitude]) > MinAmplitude;

	//check true amplitude
	double highest = -1e9;
	double lowest = 1e9;
	for (size_t i = 0; i < m_x.size(); i++)
	{
		double d = base_params[Offset] + base_params[Amplitude] * RamseyProbability(omega(m_x[i]),  M_PI / base_params[OmegaR], base_params[OmegaR], base_params[Omega0], RamseyT);
		highest = std::max(highest, d);
		lowest = std::min(lowest, d);
	}
	double trueAmplitude = highest - lowest;
	cout << "true amplitude = " << trueAmplitude << endl;
	IsGoodFit &=  trueAmplitude > MinAmplitude;


	//check fitted contrast
	double contrast = base_params[Amplitude] > 0 ? base_params[Amplitude] / (base_params[Amplitude] + base_params[Offset]) : fabs(base_params[Amplitude]) / base_params[Offset];
	cout << "fitted contrast = " << contrast << endl;
	IsGoodFit &=  contrast > MinContrast;

	//check true contrast
	lowest = *min_element(m_y.begin(), m_y.end());
	highest = *max_element(m_y.begin(), m_y.end());
	double trueContrast = 1 - lowest / highest;
	cout << "true contrast = " << trueContrast << endl;
	IsGoodFit &=  trueContrast > MinContrast;

	//make sure the peak is within the scan
	IsGoodFit &= ( base_params[Omega0] / (2 * M_PI) ) > *min_element(m_x.begin(), m_x.end());
	IsGoodFit &= ( base_params[Omega0] / (2 * M_PI) ) < *max_element(m_x.begin(), m_x.end());

	cout << ( IsGoodFit ? "Good fit :-)" : "Bad fit :-(" ) << endl;

	return IsGoodFit;
}


void FitRamseyFreq::UpdateParams()
{
	base_params[Omega0] += base_params[OmegaRange] * fit_params[Omega0];
	base_params[OmegaR] *= fit_params[OmegaR];
	base_params[Amplitude] *= fit_params[Amplitude];
	base_params[Offset] *= fit_params[Offset];
	base_params[T] *= fit_params[T];

	fill(fit_params.begin(), fit_params.end(), 1);
	fit_params[Omega0] = 0;
}


void FitRamseyFreq::PrintParams(ostream* os)
{
	*os << "FitRamseyFreq params" << endl;

	*os << scientific << setprecision(9);
	*os << setw(12) << "f0 = " << GetCenter()  << endl;
	*os << setw(12) << "omega0 = " << base_params[Omega0] << endl;
	*os << setw(12) << "omegaR = " << base_params[OmegaR]  << endl;
	*os << setw(12) << "amplitude = " << base_params[Amplitude] << endl;
	*os << setw(12) << "offset = " << base_params[Offset]  << endl;
	*os << setw(12) << "t = pi/omegaR" << endl;
	*os << setw(12) << "T = " << base_params[T] << endl;
}

double FitRamseyFreq::GetCenter()
{
	return base_params[Omega0] / (2 * M_PI);
}

void FitRamseyFreq::fitfunction(const double* params,
                                const std::vector<double>& x,
                                std::vector<double>& f_of_x)
{
	double l_offset = this->base_params[Offset] * params[Offset];
	double l_amplitude = this->base_params[Amplitude] * params[Amplitude];
	double l_omegaR = this->base_params[OmegaR] * params[OmegaR];
	double l_omega0 = this->base_params[Omega0] + base_params[OmegaRange] * params[Omega0];
	double l_T = base_params[T] * params[T];

	for (size_t i = 0; i < x.size(); i++)
		f_of_x[i] = l_offset +
		            l_amplitude * RamseyProbability(omega(x[i]), M_PI / l_omegaR, l_omegaR, l_omega0, l_T);
}

void FitRamseyFreq::UpdateScanPage(ExpSCAN* pScanPage)
{
	pScanPage->UpdateFittedCenter(GetCenter(), GoodFit());
}

