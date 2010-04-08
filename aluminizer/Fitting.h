#pragma once

class data_plot;
class ExpSCAN;


namespace numerics
{

class FitObject
{
public:
FitObject(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, std::string output_dir);

virtual ~FitObject() ;

//perform fit and return whether or not the fit object now contains valid data
virtual bool DoFit();

void PlotFit(data_plot* p, double xOffset = 0, double xScale = 1);

bool GoodFit()
{
	return IsGoodFit;
}

//use by LM_LeastSquare
void fcn(const double* x, double* fvec);

//fill f_of_x with the fit function values evaluated at x for the spec'd parameters
virtual void fitfunction(const double* params,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x) = 0;

virtual void UpdateScanPage(ExpSCAN*) = 0;

virtual size_t BaseParamsSize() const = 0;
virtual size_t FitParamsSize() const = 0;

virtual size_t MinDataPoints() const
{
	return 1;
}

double get_fitYmin()
{
	return fitYmin;
}
double get_fitYmax()
{
	return fitYmax;
}

protected:
virtual bool HasValidData();

virtual bool CheckFit() = 0;

virtual void UpdateParams() = 0;

virtual void PrintParams(std::ostream* os) = 0;

//allow for different initial parameter guesses, and automatically decide which one was best
//in order to make fitting more robust
class ParamsGuess {
public:
virtual ~ParamsGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*) = 0;
virtual std::vector<double> InitialFitParams(FitObject*) = 0;

std::vector<double> base_params;
std::vector<double> fit_params;

double EuclideanNorm;
};

ParamsGuess* BetterGuess(ParamsGuess* g1, ParamsGuess* g2);

private:
std::vector< std::vector<double> >* m_xy;



protected:
std::vector<double> m_x;
std::vector<double> m_y;
std::vector<double> m_yfit;

std::vector<double> xb;
std::vector<double> yb;

std::vector<ParamsGuess*> Guesses;
std::vector<double> fit_params;
std::vector<double> base_params;

bool IsGoodFit;

std::string output_dir;

std::vector<double> xfit, yfit;
double fitYmin, fitYmax, dx;
};

class PeakFitObject
{
public:
virtual ~PeakFitObject()
{
};
virtual double GetCenter() = 0;
};

class TimeFitObject
{
public:
virtual ~TimeFitObject()
{
};
virtual double GetPiTime() = 0;
};

class FitLorentzian : public FitObject, public PeakFitObject
{
public:
FitLorentzian(std::vector< std::vector<double> >* xy,
              size_t x_column, size_t y_column,
              double MinContrast,
              double MinAmplitude,
              std::string output_dir);

virtual ~FitLorentzian()
{
};

virtual void fitfunction(const double* params,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

double GetCenter();

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

virtual size_t BaseParamsSize() const;
virtual size_t FitParamsSize() const;
virtual size_t MinDataPoints() const;

public:

enum { Center, Amplitude, Offset, FWHM, CenterRange };

double MinContrast;
double MinAmplitude;

protected:

class NormalGuess : public ParamsGuess
{
public:
virtual ~NormalGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};
};

class FitRabiFreq : public FitObject, public PeakFitObject
{
public:
FitRabiFreq(std::vector< std::vector<double> >* xy,
            size_t x_column, size_t y_column,
            double MinContrast,
            double MinAmplitude,
            double InterrogationTime,
            std::string output_dir);
virtual ~FitRabiFreq()
{
};

virtual void fitfunction(const double* params,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

double GetCenter();

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

virtual size_t BaseParamsSize() const;
virtual size_t FitParamsSize() const;
virtual size_t MinDataPoints() const;

public:

enum { Omega0, OmegaR, Amplitude, Offset, T, OmegaRange };

double MinContrast;
double MinAmplitude;
double InterrogationTime;

protected:

class NormalGuess : public ParamsGuess
{
public:
virtual ~NormalGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};
};

class FitRamseyFreq : public FitObject, public PeakFitObject
{
public:
FitRamseyFreq(std::vector< std::vector<double> >* xy,
              size_t x_column, size_t y_column,
              double MinContrast, double MinAmplitude,
              int direction, double omegaR, double RamseyT, std::string output_dir);

virtual ~FitRamseyFreq()
{
};

virtual void fitfunction(const double* params,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

double GetCenter();

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

public:

enum { Omega0, OmegaR, Amplitude, Offset, T, OmegaRange };

double MinContrast;
double MinAmplitude;
double RamseyT;
int direction;
double omegaR;

protected:

virtual size_t BaseParamsSize() const
{
	return 6;
}
virtual size_t FitParamsSize() const
{
	return 5;
}
virtual size_t MinDataPoints() const
{
	return 10;
}

class NormalGuess : public ParamsGuess
{
public:
virtual ~NormalGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};

class PeakGuess : public ParamsGuess
{
public:
virtual ~PeakGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};
};

class FitRabiTime : public FitObject, public TimeFitObject
{
public:
FitRabiTime(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, double MinContrast, double MinAmplitude, std::string output_dir);
virtual ~FitRabiTime()
{
};

virtual void fitfunction(const double* params,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

double GetPiTime();
double GetPhase();
double GetInitialDelay();

void GuessParams(int i);

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

public:

enum { OmegaR, Phase, Amplitude, Offset, Tau, Asym };

double MinContrast;
double MinAmplitude;

virtual size_t BaseParamsSize() const
{
	return 6;
}
virtual size_t FitParamsSize() const
{
	return 6;
}
virtual size_t MinDataPoints() const
{
	return 10;
}

protected:

class NormalGuess : public ParamsGuess
{
public:
virtual ~NormalGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};

class LongGuess : public ParamsGuess
{
public:
virtual ~LongGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};

class FFTGuess : public ParamsGuess
{
public:
virtual ~FFTGuess()
{
}
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};

};

class FitLine : public FitObject
{
public:
FitLine(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, std::string output_dir);
virtual ~FitLine()
{
};

virtual void fitfunction(const double* parsms,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

// y(x) = slope * x + offset
double GetSlope() const;
double GetOffset() const;

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

virtual size_t BaseParamsSize() const
{
	return 2;
}
virtual size_t FitParamsSize() const
{
	return 2;
}
virtual size_t MinDataPoints() const
{
	return 3;
}

public:

enum { Offset, Slope };

protected:

class NormalGuess : public ParamsGuess
{
public:
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};
};

class FitRepump : public FitObject
{
public:
FitRepump(std::vector< std::vector<double> >* xy, size_t x_column, size_t y_column, std::string output_dir);
virtual ~FitRepump()
{
};

virtual void fitfunction(const double* parsms,
                         const std::vector<double>& x,
                         std::vector<double>& f_of_x);

virtual void PrintParams(std::ostream* os);
virtual void UpdateParams();

virtual bool CheckFit();

// y(x) = BkgRate * x + Amplitude * ( (x + tau * exp(-x/tau) ) - tau )
double GetBkgRate() const;
double GetTau() const;
double GetBrightRate() const;

virtual void UpdateScanPage(ExpSCAN* _pScanPage);

virtual size_t BaseParamsSize() const
{
	return 3;
}
virtual size_t FitParamsSize() const
{
	return 3;
}
virtual size_t MinDataPoints() const
{
	return 4;
}

public:

enum { BkgRate, tau, BrightRate };

protected:

class NormalGuess : public ParamsGuess
{
public:
virtual std::vector<double> GuessBaseParams(FitObject*);
virtual std::vector<double> InitialFitParams(FitObject*);
};
};

} //namespace numerics

