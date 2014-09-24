#pragma once

#include "EasyBMP\\EasyBMP.h"

class EasyGraph : public BMP
{
public:
	EasyGraph(void);
	~EasyGraph(void);
	void SetBGColor(unsigned int r, unsigned int g, unsigned int b);
	void PlotDBSpectrogram(double const * FirstValPtr, size_t Stride, unsigned int Width, unsigned int Height, double AttenuationCutOff);
	void PlotArrayIstogram(double const * FirstValPtr, size_t Stride, unsigned int NumVals, double PixelYAxisDelta, float XAxisPos, unsigned int r, unsigned int g,unsigned int b, bool AddColors);
	void PlotArrayIstogram(float const * FirstValPtr, size_t Stride, unsigned int NumVals, float PixelYAxisDelta, float XAxisPos, unsigned int r, unsigned int g,unsigned int b, bool AddColors);
};
