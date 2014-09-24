#pragma once

#include <windows.h>

class Timer
{
public:
	Timer(void);
	bool QuerySupport(void) const;
	bool SetTimeBase(void);
	float GetElapsedMsecs(void);
	bool HavePassed(float Msecs) const;

private:
	bool m_bSupported;
	LARGE_INTEGER m_iFrequency, m_iBaseCount; 
};
