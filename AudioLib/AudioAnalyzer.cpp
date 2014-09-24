#include "AudioAnalyzer.h"

#include <cmath>
#include "AudioFeatures.h"
#include "fftw3.h"
#include "PCMWAV.h"

AudioAnalyzer::AudioAnalyzer(void):
m_dMathPI(2.0*acos(0.0)),
m_eWindowFuncToUse(HANN),
m_fWndLenInSecs(0.1f),
m_bUsePadding(true),
m_fOverlap(0.5f)
{
}

AudioAnalyzer::~AudioAnalyzer(void)
{
}

void AudioAnalyzer::SetWindowingFunction(WindowFunctionEnum Function)
{
	m_eWindowFuncToUse = Function;
}

void AudioAnalyzer::SetWindowLen(float WindowLenInSecs)
{
	m_fWndLenInSecs = WindowLenInSecs;
}
	
void AudioAnalyzer::SetOverlap(float NormalizedOverlap)
{
	m_fOverlap = NormalizedOverlap;
}
	
void AudioAnalyzer::UsePadding(bool UsePadding)
{
	m_bUsePadding = UsePadding;
}

void AudioAnalyzer::ComputeFFT(const PCMWAV& Input, unsigned int ChannelID, FFTWOutput& Output)
{
	unsigned int i,j;
	//Calculate transient values
	ComputeTransientValues(Input);

	//Copy transient parameter to output
	Output.NumFreqBins = m_uiNumFreqBins;
	Output.NumWindows = m_uiNumWindows;
	Output.SampleRate = m_uiSampleRate;
	Output.WndLenInSamples = m_uiWndLenInSamples;
	Output.OverlapInSamples = m_uiOverlapInSamples;

	//Create the output arrays
	if(Output.WindowPowersPtr!=NULL)
	{
		delete[] Output.WindowPowersPtr;
		Output.WindowPowersPtr = NULL;
	}
	Output.WindowPowersPtr = new double[m_uiNumWindows];
	if(Output.OutputPtr!=NULL)
	{
		delete[] Output.OutputPtr;
		Output.OutputPtr = NULL;
	}
	Output.OutputPtr = new fftw_complex[m_uiNumWindows*m_uiNumFreqBins];

	//Set up FFTW
	double *Tempinput;
	fftw_complex *Tempoutput;
	fftw_plan FFTWPlan;
	Tempinput = (double*) fftw_malloc(sizeof(double) * m_uiPaddedWndLenInSamples);
	Tempoutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_uiNumFreqBins);
	FFTWPlan = fftw_plan_dft_r2c_1d(m_uiPaddedWndLenInSamples,Tempinput,Tempoutput,FFTW_MEASURE);
	memset(Tempinput,0x0,sizeof(double)*m_uiPaddedWndLenInSamples);

	//Perform Discrete Fourier Transform
	unsigned int TempIndex;
	for(i=0;i<m_uiNumWindows;i++)
	{
		//Populate input array with samples from current window
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			TempIndex = i*m_uiWndStride+j;
			if(TempIndex < m_uiSamplesPerChannel)
			{
				//scale input signal so that range is between -1 and +1
				Tempinput[j] = double(Input.GetSample(ChannelID,TempIndex));
			}
			else
			{
				Tempinput[j] = 0;
			}
		}
		//Calculate the current window's power and store it in the output
		Output.WindowPowersPtr[i] = 0.0;
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			Output.WindowPowersPtr[i]+=pow(Tempinput[j],2.0);
		}
		//scale input signal so that range is between -1 and +1
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			Tempinput[j] = Tempinput[j]/32767.0;
		}
		//Apply window function
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			switch(m_eWindowFuncToUse)
			{
			case HANN:
				Tempinput[j] = Tempinput[j] * (0.5 - (0.5 * cos(2.0*m_dMathPI*double(j)/double(m_uiWndLenInSamples-1))));
				break;
			case HAMMING:
				Tempinput[j] = Tempinput[j] * (0.53836 - (0.46164 * cos(2.0*m_dMathPI*double(j)/double(m_uiWndLenInSamples-1))));
				break;
			}
		}

		//Execute fourier transform
		fftw_execute(FFTWPlan);
		
		//Normalizing output of fourier transform:
		for(j=0;j<m_uiNumFreqBins;j++)
		{
			//As input signal is purely real, its spectrum is hemitian, i.e. symmetric with respect to the origin 0Hz, or DC "frequency"; 
			//to save time only the positive half of the spectrum is computed.
			//Additionally, the computed fourier transform is unnormalized: the coefficients are left proportional to the number of samples analyzed (the window length)
			//Results are therefore divided by the window length and doubled
			Tempoutput[j][0] = 2.0*Tempoutput[j][0]/double(m_uiPaddedWndLenInSamples);
			Tempoutput[j][1] = 2.0*Tempoutput[j][1]/double(m_uiPaddedWndLenInSamples);
		}

		//Store result in output array
		memcpy((void*)(Output.OutputPtr+(i*m_uiNumFreqBins)),(void*)Tempoutput,(sizeof(fftw_complex)*m_uiNumFreqBins));
	}


	//Release temporary arrays
	fftw_destroy_plan(FFTWPlan);
	fftw_free(Tempinput);
	fftw_free(Tempoutput);

	//Conversion to amplitude and phase information, if requested
	fftw_complex TempVal;
	for(i=0;i<m_uiNumWindows;i++)
	{
		for(j=0;j<m_uiNumFreqBins;j++)
		{
			TempVal[0] = Output.Element(i,j)[0];
			TempVal[1] = Output.Element(i,j)[1];
			ConvertToAmpliPhase(TempVal);
			Output.Element(i,j)[0] = TempVal[0];
			Output.Element(i,j)[1] = TempVal[1];
		}
	}

	//Phase unwrapping (start at element 2 as el 0 is DC and has no phase; end at first to last element for the same reason)
	for(i=0;i<m_uiNumWindows;i++)
	{
		UnwrapArrayPhase(&Output.Element(i,1)[1],sizeof(fftw_complex),m_uiNumFreqBins-2);
	}
}

void AudioAnalyzer::ComputeAudioFeatures(FFTWOutput& Input, AudioFeatures& Output)
{
	unsigned int i;
	Output.CalculateGlobalParams(Input.SampleRate,Input.WndLenInSamples,Input.OverlapInSamples);
	Output.SetNumWindows(Input.NumWindows);
	//Calculate features values
	for(i=0;i<Output.GetNumWindows();i++)
	{
		Output.Window(i).Power = Input.WindowPowersPtr[i];
		Output.Window(i).FreqCentroid = ComputeArraySpectralCentroid(Input.OutputPtr+(i*Input.NumFreqBins),Input.NumFreqBins);
		Output.Window(i).FreqSpread = ComputeArraySpectralSpread(Input.OutputPtr+(i*Input.NumFreqBins),Input.NumFreqBins,Output.Window(i).FreqCentroid);
		Output.Window(i).GroupDelay = Input.Element(i,Input.NumFreqBins-2)[1];
	}
	//Calculate feature deltas
	Output.CalculateDeltas();
	//Normalize feature deltas
	Output.NormalizeDeltas();
	//Normalize group delays
	Output.NormalizeGroupDelays();
	//Calculate rhythm interpolation
	Output.CalculateRhythmInterpolation();
}

void AudioAnalyzer::ComputeAudioFeatures(const PCMWAV& Input, unsigned int ChannelID, AudioFeatures& Output)
{
	unsigned int i,j;
	//Calculate transient values
	ComputeTransientValues(Input);

	//Calculate Output global parameters
	Output.CalculateGlobalParams(m_uiSampleRate,m_uiWndLenInSamples,m_uiOverlapInSamples);
	Output.SetNumWindows(m_uiNumWindows);

	//Set up FFTW
	double *Tempinput;
	fftw_complex *Tempoutput;
	fftw_plan FFTWPlan;
	Tempinput = (double*) fftw_malloc(sizeof(double) * m_uiPaddedWndLenInSamples);
	Tempoutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_uiNumFreqBins);
	FFTWPlan = fftw_plan_dft_r2c_1d(m_uiPaddedWndLenInSamples,Tempinput,Tempoutput,FFTW_MEASURE);
	memset(Tempinput,0x0,sizeof(double)*m_uiPaddedWndLenInSamples);

	//Perform Discrete Fourier Transform
	unsigned int TempIndex;
	for(i=0;i<m_uiNumWindows;i++)
	{
		//Populate input array with samples from current window
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			TempIndex = i*m_uiWndStride+j;
			if(TempIndex < m_uiSamplesPerChannel)
			{
				//scale input signal so that range is between -1 and +1
				Tempinput[j] = double(Input.GetSample(ChannelID,TempIndex));
			}
			else
			{
				Tempinput[j] = 0;
			}
		}
		//Calculate the current window's power and store it in the output
		Output.Window(i).Power = 0.0;
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			Output.Window(i).Power += pow(Tempinput[j],2.0);
		}
		//scale input signal so that range is between -1 and +1
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			Tempinput[j] = Tempinput[j]/32767.0;
		}
		//Apply window function
		for(j=0;j<m_uiWndLenInSamples;j++)
		{
			switch(m_eWindowFuncToUse)
			{
			case HANN:
				Tempinput[j] = Tempinput[j] * (0.5 - (0.5 * cos(2.0*m_dMathPI*double(j)/double(m_uiWndLenInSamples-1))));
				break;
			case HAMMING:
				Tempinput[j] = Tempinput[j] * (0.53836 - (0.46164 * cos(2.0*m_dMathPI*double(j)/double(m_uiWndLenInSamples-1))));
				break;
			}
		}

		//Execute fourier transform
		fftw_execute(FFTWPlan);
		
		//Normalizing output of fourier transform:
		for(j=0;j<m_uiNumFreqBins;j++)
		{
			//As input signal is purely real, its spectrum is hemitian, i.e. symmetric with respect to the origin 0Hz, or DC "frequency"; 
			//to save time only the positive half of the spectrum is computed.
			//Additionally, the computed fourier transform is unnormalized: the coefficients are left proportional to the number of samples analyzed (the window length)
			//Results are therefore divided by the window length and doubled
			Tempoutput[j][0] = 2.0*Tempoutput[j][0]/double(m_uiPaddedWndLenInSamples);
			Tempoutput[j][1] = 2.0*Tempoutput[j][1]/double(m_uiPaddedWndLenInSamples);
		}

		//Conversion to amplitude and phase information, if requested
		for(j=0;j<m_uiNumFreqBins;j++)
		{
			ConvertToAmpliPhase(Tempoutput[j]);
		}
		
		//Phase unwrapping
		UnwrapArrayPhase(&Tempoutput[1][1],sizeof(fftw_complex),m_uiNumFreqBins-2);

		Output.Window(i).FreqCentroid = ComputeArraySpectralCentroid(Tempoutput,m_uiNumFreqBins);
		Output.Window(i).FreqSpread = ComputeArraySpectralSpread(Tempoutput,m_uiNumFreqBins,Output.Window(i).FreqCentroid);
		Output.Window(i).GroupDelay = Tempoutput[m_uiNumFreqBins-2][1];
	}


	//Release temporary arrays
	fftw_destroy_plan(FFTWPlan);
	fftw_free(Tempinput);
	fftw_free(Tempoutput);


	//Calculate feature deltas
	Output.CalculateDeltas();
	//Normalize feature deltas
	Output.NormalizeDeltas();
	//Normalize group delays
	Output.NormalizeGroupDelays();
	//Calculate rhythm interpolation
	Output.CalculateRhythmInterpolation();
}

void AudioAnalyzer::ComputeTransientValues(const PCMWAV& Input)
{
	//Copy sampling frequency to output (could come in handy)
	m_uiSampleRate = Input.GetSampleRate();
	//Determine the window length in samples
	m_uiWndLenInSamples = unsigned int(floor((m_fWndLenInSecs*float(Input.GetSampleRate()))+0.5f));
	//Determine the overlap in samples (function of window length and overlap)
	m_uiOverlapInSamples = unsigned int (float(m_uiWndLenInSamples)*m_fOverlap);
	//Determine the window stride (windowlen - overlap)
	m_uiWndStride = m_uiWndLenInSamples - m_uiOverlapInSamples;

	//Calculate number of windows
	m_uiSamplesPerChannel = Input.GetNumSamples()/Input.GetNumChannels();
	if(m_uiSamplesPerChannel <= m_uiWndLenInSamples)
	{
		m_uiNumWindows = 1;
	}
	else
	{
		m_uiNumWindows = ((m_uiSamplesPerChannel-m_uiWndLenInSamples)/m_uiWndStride);
		if((m_uiSamplesPerChannel-m_uiWndLenInSamples)%m_uiWndStride!=0)
		{
			m_uiNumWindows++;
		}
		m_uiNumWindows++;
	}

	//Padded window length:
	m_uiPaddedWndLenInSamples = 1;
	//If padding has been disabled by the user, the padded window length is equal to the actual one
	if(m_bUsePadding==false)
	{
		m_uiPaddedWndLenInSamples = m_uiWndLenInSamples;
	}
	else
	{
		//Otherwise, calculate the padded window's length so that it's equal to the smallest power of 2 greater than the actual window's length
		while(m_uiPaddedWndLenInSamples < m_uiWndLenInSamples)
		{
			m_uiPaddedWndLenInSamples = m_uiPaddedWndLenInSamples*2;
		}
	}

	//Calculate the number of output frequency bins (this is a function of the length of the input to the FFT, in this case it's the length of the padded window)
	m_uiNumFreqBins = (m_uiPaddedWndLenInSamples/2)+1;
}

//Converts a complex number into polar form
void AudioAnalyzer::ConvertToAmpliPhase(fftw_complex& Value)
{
	double Amplitude,Phase;

	Amplitude = sqrt(pow(Value[0],2.0)+pow(Value[1],2.0));
	Phase = atan(Value[1]/Value[0]);
	if(Value[0] == 0.0)
	{
		if(Value[1] < 0.0)
		{
			Phase = 0.0 - (m_dMathPI/2.0);
		}
		else if(Value[1] == 0.0)
		{
			Phase = 0.0;
		}
		else
		{
			Phase = (m_dMathPI/2.0);
		}
	}
	else if(Value[0] < 0.0)
	{
		if(Value[1] < 0.0)
		{
			Phase -= m_dMathPI;
		}
		else
		{
			Phase += m_dMathPI;
		}
	}
	Value[0] = Amplitude;
	Value[1] = Phase;
}

//Unwraps the phase of an array of double values with custom stride
void AudioAnalyzer::UnwrapArrayPhase(double* StartValPtr, size_t Stride, unsigned int NumVals)
{
	unsigned int i;
	double *PtrCurr,*PtrPrev;
	double CurrVal, PrevVal;

	for(i=1;i<NumVals;i++)
	{
		PtrCurr=(double*)(((char*)StartValPtr)+(i*Stride));
		PtrPrev=(double*)(((char*)StartValPtr)+((i-1)*Stride));
		CurrVal=*PtrCurr;
		PrevVal=*PtrPrev;
		if(PrevVal-CurrVal > m_dMathPI)
		{
			CurrVal += (2.0*m_dMathPI)*floor((PrevVal-CurrVal)/(2.0*m_dMathPI));
			if(PrevVal-CurrVal > m_dMathPI)
			{
				CurrVal += (2.0*m_dMathPI);
			}
		}
		else if(CurrVal-PrevVal > m_dMathPI)
		{
			CurrVal -= (2.0*m_dMathPI)*floor((CurrVal-PrevVal)/(2.0*m_dMathPI));
			if(CurrVal-PrevVal > m_dMathPI)
			{
				CurrVal -= (2.0*m_dMathPI);
			}
		}
		*PtrCurr=CurrVal;
	}
}

//Compute the power of an array of values with custom stride as if they were representing samples of a waveform
double AudioAnalyzer::ComputeArrayPower(double* StartValPtr, size_t Stride, unsigned int NumVals)
{
	unsigned int i;
	double Result = 0.0;
	for(i=0;i<NumVals;i++)
	{
		Result += pow(*(double*)(((char*)StartValPtr)+(i*Stride)),2.0);
	}
	return Result;
}

//Computes the normalized spectral centroid for an array of complex values in amplitude/phase form
//Actual value is equal to normalized one times nyquist freq (half of sample rate)
double AudioAnalyzer::ComputeArraySpectralCentroid(fftw_complex* StartValPtr, unsigned int NumVals)
{
	unsigned int i;
	double TotalPow,HalfPow,LHSPow,FractionalPart,Res;
	
	TotalPow= 0.0;
	for(i=0;i<NumVals;i++)
	{
		TotalPow += pow(StartValPtr[i][0],2.0);
	}
	if(TotalPow <= 0.0)
	{
		return 0.0;
	}

	HalfPow = TotalPow/2.0;

	LHSPow = 0.0;
	i=0;
	LHSPow += pow(StartValPtr[0][0],2.0);
	while(LHSPow < HalfPow)
	{
		i++;
		LHSPow += pow(StartValPtr[i][0],2.0);
	}
	LHSPow -= pow(StartValPtr[i][0],2.0);
	FractionalPart = (HalfPow - LHSPow)/(pow(StartValPtr[i][0],2.0));
	Res = double(i)+FractionalPart;
	Res = Res/double(NumVals);
	return Res;
}

//Computes the normalized spectral spread about NormalizedCenterFrequency
double AudioAnalyzer::ComputeArraySpectralSpread(fftw_complex* StartValPtr, unsigned int NumVals, double NormalizedCenterFrequency)
{
	unsigned int i;
	double Result = 0.0;
	for(i=0;i<NumVals;i++)
	{
		Result += (pow(StartValPtr[i][0],2.0))*fabs(((double(i)+0.5)/double(NumVals))-NormalizedCenterFrequency);
	}
	return Result;
}

//Normalize an array of double type values so that they lie between -1 and 1 while maintaining their proportionality
void AudioAnalyzer::NormalizeArray(double* StartValPtr, size_t Stride, unsigned int NumVals)
{
	unsigned int i;
	double MaxAbsVal = 0.0;
	double* CurrElPtr;
	for(i=0;i<NumVals;i++)
	{
		CurrElPtr = (double*)(((char*)StartValPtr)+(i*Stride));
		if(fabs(*CurrElPtr) > MaxAbsVal)
		{
			MaxAbsVal = fabs(*CurrElPtr);
		}
	}
	for(i=0;i<NumVals;i++)
	{
		CurrElPtr = (double*)(((char*)StartValPtr)+(i*Stride));
		*CurrElPtr = (*CurrElPtr)/MaxAbsVal;
	}
}