#pragma once

#include "FFTWOutput.h"

class AudioFeatures;
class PCMWAV;

class AudioAnalyzer
{
public:
	enum WindowFunctionEnum {HANN, HAMMING};

	AudioAnalyzer(void);
	~AudioAnalyzer(void);
	//Parameter setting accessors
	void SetWindowingFunction(WindowFunctionEnum Function);
	void SetWindowLen(float WindowLenInSecs);
	void SetOverlap(float NormalizedOverlap);
	void UsePadding(bool UsePadding);
	//Main computing functions
	void ComputeFFT(const PCMWAV& Input, unsigned int ChannelID, FFTWOutput& Output);
	void ComputeAudioFeatures(FFTWOutput& Input, AudioFeatures& Output);
	void ComputeAudioFeatures(const PCMWAV& Input, unsigned int ChannelID, AudioFeatures& Output);

protected:
	//Copy constructor, here to make sure users don't copy the class
	AudioAnalyzer(const AudioAnalyzer&);
	//Internal usage auxiliary functions
	void ComputeTransientValues(const PCMWAV& Input);
	void ConvertToAmpliPhase(fftw_complex& Value);
	void UnwrapArrayPhase(double* StartValPtr, size_t Stride, unsigned int NumVals);
	double ComputeArrayPower(double* StartValPtr, size_t Stride, unsigned int NumVals);
	double ComputeArraySpectralCentroid(fftw_complex* StartValPtr, unsigned int NumVals);
	double ComputeArraySpectralSpread(fftw_complex* StartValPtr, unsigned int NumVals, double NormalizedCenterFrequency);
	void NormalizeArray(double* StartValPtr, size_t Stride, unsigned int NumVals);
	//Transformation settings
	WindowFunctionEnum m_eWindowFuncToUse;
	float m_fWndLenInSecs;
	float m_fOverlap;
	bool m_bUsePadding;
	//Transient values (they change for every input file processed)
	unsigned int m_uiSamplesPerChannel;
	unsigned int m_uiNumWindows;
	unsigned int m_uiWndLenInSamples;
	unsigned int m_uiPaddedWndLenInSamples;
	unsigned int m_uiWndStride;
	unsigned int m_uiOverlapInSamples;
	unsigned int m_uiNumFreqBins;
	unsigned int m_uiSampleRate;
	//PI
	const double m_dMathPI;
};
