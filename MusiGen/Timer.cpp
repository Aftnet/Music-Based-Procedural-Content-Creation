#include "Timer.h"

Timer::Timer(void)
{
	if(QueryPerformanceFrequency(&m_iFrequency) != 0)
	{
		m_bSupported = true;
	}
	else
	{
		m_bSupported = false;
	}

	SetTimeBase();
}

bool Timer::QuerySupport(void) const
{
	return m_bSupported;
}

bool Timer::SetTimeBase(void)
{
	if(m_bSupported == true)
	{
		if(QueryPerformanceCounter(&m_iBaseCount) != 0)
		{
			return true;
		}
	}
	return false;
}

float Timer::GetElapsedMsecs(void)
{
	LARGE_INTEGER CurrentCount;
	
	if(m_bSupported == true)
	{
		if(QueryPerformanceCounter(&CurrentCount) != 0)
		{
			CurrentCount.QuadPart -= m_iBaseCount.QuadPart;
			return (1000.0f*float(CurrentCount.QuadPart)/float(m_iFrequency.QuadPart));
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		return 0.0f;
	}
}

bool Timer::HavePassed(float Msecs) const
{
	LARGE_INTEGER CurrentCount;
	if(QueryPerformanceCounter(&CurrentCount) != 0)
	{
		CurrentCount.QuadPart -= m_iBaseCount.QuadPart;
		if(((1000.0f*float(CurrentCount.QuadPart))/float(m_iFrequency.QuadPart)) > Msecs)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
