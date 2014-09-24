#pragma once

#include <fftw3.h>

class FFTWOutput
{
public:
	FFTWOutput(void);
	~FFTWOutput(void);
	double* const Element(unsigned int Window, unsigned int FreqBin);

	unsigned int SampleRate;
	unsigned int WndLenInSamples;
	unsigned int OverlapInSamples;
	unsigned int NumFreqBins;
	unsigned int NumWindows;
	double* WindowPowersPtr;
	fftw_complex* OutputPtr;

protected:
	FFTWOutput(const FFTWOutput&);
};
