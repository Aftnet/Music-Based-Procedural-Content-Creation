#pragma once

#include <vector>
#include "AudioFeatures.h"

struct AudioSectionStr
{
	double AvgPower,AvgCentroid,AvgSpread;
	float BPM;
	std::vector<unsigned int>BeatLocations;
};


class AudioSections
{
public:
	AudioSections(void);
	~AudioSections(void);
	unsigned int GetNumSections(void) const;
	float GetSectionLengthInMsecs(void) const;
	unsigned int GetSectionLengthInWindows(void) const;
	float GetGlobalBPM(void) const;
	AudioSectionStr& Section(unsigned int Index);
	AudioSectionStr* GetSectionsArrPtr(void);
	void Generate(AudioFeatures& Input, float SectionLenInMsecs);
	

protected:
	AudioSections(const AudioSections&);
	double ArrayMean(double* StartValPtr, size_t Stride, unsigned int NumVals);
	double ArrayStandardDev(double* StartValPtr, size_t Stride, unsigned int NumVals, double Mean);
	void ComputeGlobalBPM(void);

	struct IntervalStruct
	{
		unsigned int Len,StartBeat,EndBeat;
	};

	struct IntervalFreqStruct
	{
		IntervalFreqStruct(void);
		IntervalFreqStruct& operator=(const IntervalFreqStruct& rhs);
		unsigned int Len,Occurrences,MaxChainLen,MaxChainNumEl;
		float Weight;
		std::vector<IntervalStruct> MemberIntervals;
	};


	float m_fSectionLengthInMsecs;
	unsigned int m_uiSectionLengthInWindows;
	unsigned int m_uiNumSections;
	float m_fGlobalBPM;
	AudioSectionStr* m_pSectionsArr;
};
