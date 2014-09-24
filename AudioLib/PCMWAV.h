#pragma once

#include <windows.h>


class PCMWAV
{
public:
	PCMWAV(void);
	~PCMWAV(void);

	unsigned int GetNumChannels(void) const;
	void SetNumChannels(unsigned int Val);
	unsigned int GetSampleRate(void) const;
	void SetSampleRate(unsigned int Val);
	unsigned int GetNumSamples(void) const;
	bool SetNumSamples(unsigned int Val);
	short GetSample(unsigned int SampleID) const;
	short GetSample(unsigned short ChannelID, unsigned int SampleID) const;
	bool SetSample(unsigned int SampleID, short Val);
	bool SetSample(unsigned short ChannelID, unsigned int SampleID, short Val);
	short * const GetDataPtr(void);
	bool RawCopy(unsigned int SamplesOffset, void* SourcePtr, unsigned int SizeInSamples);

	bool ReadFrom(LPTSTR FileName);
	bool SaveTo(LPTSTR FileName); 

protected:
	PCMWAV(const PCMWAV& Input);

	void ComputeParams(void);
	void ClearSamples(void);

	unsigned int m_uiNumSamples;
	
	int m_iSize;

	short m_siNumChannels;
	int m_iSampleRate;
	int m_iByteRate;
	short m_siBlockAlign;

	int m_iSubchunk2Size;
	short* m_pData;
};
