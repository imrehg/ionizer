#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Fitting.h"
#include "Numerics.h"
#include "ExpSCAN.h"

#include <numeric>
#include <fftw3.h>

using namespace std;
using namespace numerics;


FitLorentzian::FitLorentzian(std::vector< std::vector<double> >* xy,
                             size_t x_column, size_t y_column,
                             double MinContrast, double MinAmplitude, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir),
	MinContrast(MinContrast),
	MinAmplitude(MinAmplitude)
{
	Guesses.push_back(new NormalGuess);
}

std::vector<double> FitLorentzian::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitLorentzian* pFL = dynamic_cast<FitLorentzian*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters
	double lowest = *min_element(pFL->yb.begin(), pFL->yb.end());
	double highest = *max_element(pFL->yb.begin(), pFL->yb.end());

	base_params[CenterRange] = fabs(pFL->xb.back() - pFL->xb.front());

	base_params[Offset] = ( pFL->yb.back() + pFL->yb.front() ) / 2 ;

	//try to figure out whether we are looking at a dip or a peak.
	double direction = fabs(highest - base_params[Offset]) - fabs(lowest - base_params[Offset]);
	size_t peak = 0;

	if ( direction < 0 )
		peak = find(pFL->yb.begin(), pFL->yb.end(), lowest) - pFL->yb.begin();
	else
		peak = find(pFL->yb.begin(), pFL->yb.end(), highest) - pFL->yb.begin();

	base_params[Amplitude] = pFL->yb[peak] - base_params[Offset];
	base_params[Center] = pFL->xb[peak];

	size_t hm1 = 0;
	size_t hm2 = 0;
	double halfMax = base_params[Amplitude] / 2 + base_params[Offset];

	for (size_t i = peak; i < pFL->xb.size(); i++)
	{
		hm1 = i;

		if (direction > 0)
		{
			if (pFL->yb[i] < halfMax)
				break;
		}
		else if (pFL->yb[i] > halfMax)
			break;
	}

	for (size_t i = peak; i > 0; i--)
	{
		hm2 = i;

		if (direction > 0)
		{
			if (pFL->yb[i] < halfMax)
				break;
		}
		else if (pFL->yb[i] > halfMax)
			break;
	}

	base_params[FWHM] = fabs(pFL->xb[hm1] - pFL->xb[hm2]);

	cout << base_params[Offset] << ", " << base_params[Amplitude] << ", " << base_params[Center] << ", ";
	cout << base_params[FWHM] << endl;

	return base_params;
}

std::vector<double> FitLorentzian::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	fit_params[Center] = 0;

	return fit_params;
}

bool FitLorentzian::CheckFit()
{ /*
     if( ! HasValidData() )
      return false;

     UpdateParams();

     IsGoodFit = true;

     //check fitted amplitude
     cout << "fitted amplitude = " << fabs(base_params[Amplitude]) << endl;
     IsGoodFit &=  fabs(base_params[Amplitude]) > MinAmplitude;

     //check true amplitude
     double highest = -1e9;
     double lowest = 1e9;
     for(size_t i = 0; i < m_x.size(); i++) {
      double d = base_params[Offset] + base_params[Amplitude] * RabiProbability(omega(m_x[i]), base_params[T], base_params[OmegaR], base_params[Omega0]);
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
     IsGoodFit &= ( base_params[Omega0] / (2*M_PI) ) > *min_element(m_x.begin(), m_x.end());
     IsGoodFit &= ( base_params[Omega0] / (2*M_PI) ) < *max_element(m_x.begin(), m_x.end());

     cout << ( IsGoodFit ? "Good fit :-)" : "Bad fit :-(" ) << endl;

     return IsGoodFit; */

	return false;
}


void FitLorentzian::UpdateParams()
{
	base_params[Center] += fit_params[Center] * base_params[CenterRange];
	base_params[FWHM]   *= fit_params[FWHM];
	base_params[Amplitude] *= fit_params[Amplitude];
	base_params[Offset] *= fit_params[Offset];

	fill(fit_params.begin(), fit_params.end(), 1);
	fit_params[Center] = 0;
}


void FitLorentzian::PrintParams(ostream* os)
{
	*os << "FitLorentzian params" << endl;

	*os << fixed << setprecision(9);
	*os << "center     = " << GetCenter()  << endl;
	*os << "amplitude  = " << base_params[Amplitude]  << endl;
	*os << "offset     = " << base_params[Offset] << endl;
	*os << "FWHM       = " << base_params[FWHM] << endl;
}

double FitLorentzian::GetCenter()
{
	return base_params[Center];
}

void FitLorentzian::fitfunction(const double* params,
                                const std::vector<double>& x,
                                std::vector<double>& f_of_x)
{
	double l_offset = this->base_params[Offset] * params[Offset];
	double l_amplitude = this->base_params[Amplitude] * params[Amplitude];
	double l_FWHM = this->base_params[FWHM] * params[FWHM];
	double l_center = this->base_params[Center] + params[Center] * base_params[CenterRange];

	//don't allow fits where the peak with is less than the half of step size
	if (fabs(dx) > fabs(2 * l_FWHM))
		for (size_t i = 0; i < x.size(); i++)
			f_of_x[i] = 0;
	else
		for (size_t i = 0; i < x.size(); i++)
			f_of_x[i] = l_offset + l_amplitude / (1 + pow(2 * (x[i] - l_center) / l_FWHM, 2));
}

void FitLorentzian::UpdateScanPage(ExpSCAN* pScanPage)
{
	pScanPage->UpdateFittedCenter(GetCenter(), GoodFit());
}

size_t FitLorentzian::BaseParamsSize() const
{
	return 5;
}
size_t FitLorentzian::FitParamsSize() const
{
	return 4;
}
size_t FitLorentzian::MinDataPoints() const
{
	return 6;
}


FitRabiFreq::FitRabiFreq(std::vector< std::vector<double> >* xy,
                         size_t x_column, size_t y_column,
                         double MinContrast, double MinAmplitude, double InterrogationTime, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir),
	MinContrast(MinContrast),
	MinAmplitude(MinAmplitude),
	InterrogationTime(fabs(InterrogationTime))
{
	Guesses.push_back(new NormalGuess);
}

std::vector<double> FitRabiFreq::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRabiFreq* pFRF = dynamic_cast<FitRabiFreq*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters
	base_params[OmegaRange] = omega(pFRF->xb.back() - pFRF->xb.front());
	base_params[OmegaR] = M_PI / pFRF->InterrogationTime;
	base_params[T] = pFRF->InterrogationTime;

	double lowest = *min_element(pFRF->yb.begin(), pFRF->yb.end());
	double highest = *max_element(pFRF->yb.begin(), pFRF->yb.end());
	base_params[Offset] = ( pFRF->yb.back() + pFRF->yb.front() ) / 2 ;

	//try to figure out whether we are looking at a dip or a peak.
	double direction = fabs(highest - base_params[Offset]) - fabs(lowest - base_params[Offset]);
	size_t peak = 0;

	if ( direction < 0 )
		peak = find(pFRF->yb.begin(), pFRF->yb.end(), lowest) - pFRF->yb.begin();
	else
		peak = find(pFRF->yb.begin(), pFRF->yb.end(), highest) - pFRF->yb.begin();

	base_params[Amplitude] = pFRF->yb[peak] - base_params[Offset];
	base_params[Omega0] = 2 * M_PI * pFRF->xb[peak];

	cout << base_params[Offset] << ", " << base_params[Amplitude] << ", " << base_params[Omega0] << endl;

	return base_params;
}

std::vector<double> FitRabiFreq::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	fit_params[Omega0] = 0;

	return fit_params;
}

bool FitRabiFreq::CheckFit()
{
	if ( !HasValidData() )
		return false;

	UpdateParams();

	IsGoodFit = true;

	//check fitted amplitude
	cout << "fitted amplitude = " << fabs(base_params[Amplitude]) << endl;
	IsGoodFit &=  fabs(base_params[Amplitude]) > MinAmplitude;

	//check true amplitude
	double highest = -1e9;
	double lowest = 1e9;
	for (size_t i = 0; i < m_x.size(); i++)
	{
		double d = base_params[Offset] + base_params[Amplitude] * RabiProbability(omega(m_x[i]), base_params[T], base_params[OmegaR], base_params[Omega0]);
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


void FitRabiFreq::UpdateParams()
{
	base_params[Omega0] += base_params[OmegaRange] * fit_params[Omega0];
	base_params[OmegaR] *= fit_params[OmegaR];
//	base_params[T] *= fit_params[T];
	base_params[Amplitude] *= fit_params[Amplitude];
	base_params[Offset] *= fit_params[Offset];

	fill(fit_params.begin(), fit_params.end(), 1);
	fit_params[Omega0] = 0;
}


void FitRabiFreq::PrintParams(ostream* os)
{
	*os << "FitRabiFreq params" << endl;

	*os << fixed << setprecision(9);
	*os << "f0  = " << GetCenter()  << endl;
	*os << "wR  = " << base_params[OmegaR]  << endl;
	*os << "t   = " << base_params[T] << endl;
	*os << "A   = " << base_params[Amplitude] << endl;
	*os << "off = " << base_params[Offset]  << endl;
}

double FitRabiFreq::GetCenter()
{
	if (base_params.size() <= Omega0)
	{
		cerr << "[FitRabiFreq::GetCenterFrequency] base_params.size() <= " << Omega0 << endl;
		return 0;
	}

	return base_params[Omega0] / (2 * M_PI);
}

void FitRabiFreq::fitfunction(const double* params,
                              const std::vector<double>& x,
                              std::vector<double>& f_of_x)
{
	double l_offset = this->base_params[Offset] * params[Offset];
	double l_amplitude = this->base_params[Amplitude] * params[Amplitude];
	double l_t = this->base_params[T];
	double l_omegaR = this->base_params[OmegaR] * params[OmegaR];
	double l_omega0 = this->base_params[Omega0] + base_params[OmegaRange] * params[Omega0];

	//don't allow fits where the peak with is less than the half of step size
	if (fabs(dx) > fabs(2 / l_t))
		for (size_t i = 0; i < x.size(); i++)
			f_of_x[i] = 0;
	else
		for (size_t i = 0; i < x.size(); i++)
			f_of_x[i] = l_offset + l_amplitude * RabiProbability(omega(x[i]), l_t, l_omegaR, l_omega0);
}

void FitRabiFreq::UpdateScanPage(ExpSCAN* pScanPage)
{

	pScanPage->UpdateFittedCenter(GetCenter(), GoodFit());
}

size_t FitRabiFreq::BaseParamsSize() const
{
	return 6;
}
size_t FitRabiFreq::FitParamsSize() const
{
	return 4;
}
size_t FitRabiFreq::MinDataPoints() const
{
	return 10;
}

FitRabiTime::FitRabiTime(std::vector< std::vector<double> >* xy,
                         size_t x_column, size_t y_column,
                         double MinContrast, double MinAmplitude, string output_dir) :
	FitObject(xy, x_column, y_column, output_dir),
	MinContrast(MinContrast),
	MinAmplitude(MinAmplitude)
{
	Guesses.push_back(new FFTGuess);
//	Guesses.push_back(new NormalGuess);
//	Guesses.push_back(new LongGuess);
}

double FitRabiTime::GetPiTime()
{
	if (base_params.size() <= OmegaR)
	{
		cerr << "[FitRabiTime::GetPiTime] base_params.size() <= " << OmegaR << endl;
		return 0;
	}

	return M_PI / base_params[OmegaR];
}

double FitRabiTime::GetPhase()
{
	if (base_params.size() <= Phase)
	{
		cerr << "[FitRabiTime::GetPhase] base_params.size() <= " << Phase << endl;
		return 0;
	}

	double phase = base_params[Phase];
	if (phase > M_PI)
		phase = phase - 2 * M_PI;

	if (phase < -M_PI)
		phase = 2 * M_PI + phase;

	return phase;
}

double FitRabiTime::GetInitialDelay()
{
	if (GetPiTime() > 0)
		return GetPhase() * GetPiTime() / M_PI;
	else
		return 0;
}

bool FitRabiTime::CheckFit()
{
	UpdateParams();

	IsGoodFit = true;

	//check fitted amplitude
	cout << "fitted amplitude = " << fabs(base_params[Amplitude]) << endl;
	IsGoodFit &=  fabs(base_params[Amplitude]) > MinAmplitude;

	//check true amplitude
	double highest = -1e9;
	double lowest = 1e9;
	for (size_t i = 0; i < m_x.size(); i++)
	{
		double d = base_params[Offset] + base_params[Amplitude] * RabiFloppingProbability(m_x[i], base_params[OmegaR], base_params[Phase], base_params[Asym], base_params[Tau]);
		highest = std::max(highest, d);
		lowest = std::min(lowest, d);
	}
	double trueAmplitude = highest - lowest;
	cout << "true amplitude = " << trueAmplitude << endl;
	IsGoodFit &=  trueAmplitude > MinAmplitude;


	//check fitted contrast
	double contrast = base_params[Amplitude] > 0 ? base_params[Amplitude] / ( base_params[Amplitude] + fabs(base_params[Offset] + base_params[Amplitude] * base_params[Asym]) ) : fabs(base_params[Amplitude]) /  fabs(base_params[Offset] + base_params[Amplitude] * base_params[Asym]);
	cout << "fitted contrast = " << contrast << endl;
	IsGoodFit &=  contrast > MinContrast;

	//check true contrast
	lowest = *min_element(m_y.begin(), m_y.end());
	highest = *max_element(m_y.begin(), m_y.end());
	double trueContrast = 1 - lowest / highest;
	cout << "true contrast = " << trueContrast << endl;
	IsGoodFit &=  trueContrast > MinContrast;

	//make sure the pi-time is within the scan
	IsGoodFit &= (M_PI) / base_params[OmegaR] > *min_element(m_x.begin(), m_x.end());
	IsGoodFit &= (M_PI) / base_params[OmegaR] < *max_element(m_x.begin(), m_x.end());

	//make sure the pi-time isn't undersampled
	if (m_x.size() > 1)
	{
		cout << "t_pi = " << (M_PI) / base_params[OmegaR] << endl;
		cout << "dt = " << fabs(m_x[1] - m_x[0]) << endl;
		IsGoodFit &= (M_PI) / base_params[OmegaR] > fabs(m_x[1] - m_x[0]);
	}

	cout << ( IsGoodFit ? "Good fit :-)" : "Bad fit :-(" ) << endl;

	return IsGoodFit;
}

std::vector<double> FitRabiTime::NormalGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRabiTime* pFRT = dynamic_cast<FitRabiTime*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters

	double lowest = *min_element(pFRT->yb.begin(), pFRT->yb.end());
	double highest = *max_element(pFRT->yb.begin(), pFRT->yb.end());


	// try to find first quarter cycle time
	double mean = (highest + lowest) / 2;

	double first_crossing = -2;
	double second_crossing = -1;

	for (size_t imean = 0; (imean + 1) < pFRT->yb.size(); imean++)
	{
		if ( ( pFRT->yb[imean] > mean ) != ( pFRT->yb[imean + 1] > mean ))
		{
			if (first_crossing < 0)
				first_crossing = ( pFRT->xb[imean] + pFRT->xb[imean + 1] ) / 2;
			else
			{
				second_crossing = ( pFRT->xb[imean] + pFRT->xb[imean + 1] ) / 2;
				break;
			}
		}
	}

	base_params[OmegaR] = M_PI / fabs( second_crossing - first_crossing ) ;
	base_params[Offset] = lowest ;
	base_params[Amplitude] = highest - lowest ;
	base_params[Tau] = pFRT->xb.back() ;
	base_params[Asym] = mean / 100 ;

	//try to determine the phase
	base_params[Phase] = M_PI / 10 ;
	double meanfirst = ( pFRT->yb[0] + pFRT->yb[1] + pFRT->yb[2] + pFRT->yb[3] ) / 4.0 ;
	if ( fabs(meanfirst - lowest) > fabs(meanfirst - highest) )
		base_params[Phase] = M_PI;

	return base_params;
}

std::vector<double> FitRabiTime::NormalGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	return fit_params;
}

std::vector<double> FitRabiTime::LongGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRabiTime* pFRT = dynamic_cast<FitRabiTime*>(fit);

	base_params.resize(fit->BaseParamsSize());

	//very crude initial guess for fit parameters

	double lowest = *min_element(pFRT->yb.begin(), pFRT->yb.end());
	double highest = *max_element(pFRT->yb.begin(), pFRT->yb.end());


	// try to find first quarter cycle time
	double mean = (highest + lowest) / 2;

	base_params[OmegaR] = M_PI / fabs( pFRT->xb.back() - pFRT->xb.front() ) ;
	base_params[Offset] = lowest ;
	base_params[Amplitude] = highest - lowest ;
	base_params[Tau] = pFRT->xb.back() ;
	base_params[Asym] = mean / 100 ;

	//try to determine the phase
	base_params[Phase] = M_PI / 10 ;

	return base_params;
}

std::vector<double> FitRabiTime::LongGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	return fit_params;
}

std::vector<double> FitRabiTime::FFTGuess::GuessBaseParams(numerics::FitObject* fit)
{
	FitRabiTime* pFRT = dynamic_cast<FitRabiTime*>(fit);

	base_params.resize(fit->BaseParamsSize());

	// Do FFT on data to get initial guess for frequency
	double dx = pFRT->xb[1] - pFRT->xb[0];
	double fMax = 1 / dx;

	size_t N = pFRT->xb.size();

	double maxp2 = 0, maxp = 0, maxf = 0;

	fftw_complex *in, *out;
	fftw_plan p;

	in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

	for (size_t i = 0; i < N; i++)
	{
		in[i][0] = pFRT->yb[i];
		in[i][1] = 0;
	}

	p = fftw_plan_dft_1d((int)N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);


	if (!p)
		throw(runtime_error("[FitRabi::FFTGuess] could not create FFT plan."));

	fftw_execute(p);

	// calculate power spectrum and determine max. and 2nd max. component
	valarray<double> freq(N);
	valarray<double> power(N);


	power[0] = 0;
	freq[0] = 0;
	for (size_t i = 1; i < N / 2; i++)
	{
		power[i] = out[i][0] * out[i][0] + out[i][1] * out[i][1];
		freq[i] = fMax * (double)i / (double)N;      // in Hz (dx is in us)
		cout << freq[i] << "  " << power[i] << endl; // debug
		if ( (power[i] > maxp) | (power[i] > maxp2) )
		{
			if ( power[i] > maxp )
			{
				maxp2 = maxp;
				maxp = power[i];
				maxf = freq[i];
			}
			else
				maxp2 = power[i];
		}
	}
	cout << "Maximum frequency component = " << maxf << " MHz" << endl;

	fftw_destroy_plan(p);

	fftw_free(in);
	fftw_free(out);

	//very crude initial guess for fit parameters

	double lowest = *min_element(pFRT->yb.begin(), pFRT->yb.end());
	double highest = *max_element(pFRT->yb.begin(), pFRT->yb.end());


	// try to find first quarter cycle time
	double mean = (highest + lowest) / 2;

	base_params[OmegaR] = 2 * M_PI * maxf;
	base_params[Offset] = lowest ;
	base_params[Amplitude] = highest - lowest ;
	base_params[Tau] = pFRT->xb.back() ;
	base_params[Asym] = mean / 100 ;

	//try to determine the phase
	base_params[Phase] = M_PI / 10 ;
	double meanfirst = pFRT->yb[0];
	if ( fabs(meanfirst - lowest) > fabs(meanfirst - highest) )
		base_params[Phase] = M_PI;

	return base_params;


}

std::vector<double> FitRabiTime::FFTGuess::InitialFitParams(numerics::FitObject* fit)
{
	fit_params = vector<double>(fit->FitParamsSize(), 1);
	return fit_params;
}


void FitRabiTime::UpdateParams()
{
	base_params[OmegaR] = fabs(base_params[OmegaR] * fit_params[OmegaR]);
	base_params[Amplitude] *= fit_params[Amplitude];
	base_params[Offset] *= fit_params[Offset];
	base_params[Phase] *= fit_params[Phase];
	base_params[Tau] = fabs(base_params[Tau] * fit_params[Tau]);
	base_params[Asym] *= fit_params[Asym];

	//constrain -pi < phase <= pi;
	base_params[Phase] = fmod(base_params[Phase], 2 * M_PI);
	if (base_params[Phase] > M_PI)
		base_params[Phase] -= 2 * M_PI;

	if (base_params[Phase] <= -M_PI)
		base_params[Phase] += 2 * M_PI;

	fill(fit_params.begin(), fit_params.end(), 1);
}

void FitRabiTime::UpdateScanPage(ExpSCAN* pScanPage)
{
	pScanPage->UpdateFittedPiTime(GetPiTime(), GetInitialDelay(), GoodFit());
}

void FitRabiTime::PrintParams(ostream* os)
{
	*os << "FitRabiTime params" << endl;

	*os << setprecision(10);
	*os << setw(12) << "tpi = " <<  (M_PI) / base_params[OmegaR]  << endl;
	*os << setw(12) << "omegaR = " << base_params[OmegaR]  << endl;
	*os << setw(12) << "amplitude = " << base_params[Amplitude] << endl;
	*os << setw(12) << "offset = " << base_params[Offset]  << endl;
	*os << setw(12) << "phase = " << base_params[Phase] << endl;
	*os << setw(12) << "tau = " << base_params[Tau] << endl;
	*os << setw(12) << "asym = " << base_params[Asym] << endl;

}


void FitRabiTime::fitfunction(const double* params,
                              const std::vector<double>& x,
                              std::vector<double>& f_of_x)
{
	double l_offset = this->base_params[Offset] * params[Offset];
	double l_amplitude = this->base_params[Amplitude] * params[Amplitude];
	double l_tau = fabs(this->base_params[Tau] * params[Tau]);
	double l_omegaR = fabs(this->base_params[OmegaR] * params[OmegaR]);
	double l_phase = this->base_params[Phase] * params[Phase];
	double l_asym = this->base_params[Asym] * params[Asym];

	for (size_t i = 0; i < x.size(); i++)
		f_of_x[i] = l_offset + l_amplitude * RabiFloppingProbability(x[i], l_omegaR, l_phase, l_asym, l_tau);
}

