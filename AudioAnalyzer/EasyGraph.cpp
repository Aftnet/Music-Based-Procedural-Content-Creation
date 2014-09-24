#include "EasyGraph.h"

EasyGraph::EasyGraph(void)
{
}

EasyGraph::~EasyGraph(void)
{
}

void EasyGraph::SetBGColor(unsigned int r, unsigned int g, unsigned int b)
{
	int i,j;

	RGBApixel Color;
	Color.Red = r;
	Color.Green = g;
	Color.Blue = b;
	Color.Alpha = 0;

	for(i=0;i<TellWidth();i++)
	{
		for(j=0;j<TellHeight();j++)
		{
			SetPixel(i,j,Color);
		}
	}
}

void EasyGraph::PlotDBSpectrogram(double const * FirstValPtr, size_t Stride, unsigned int Width, unsigned int Height, double AttenuationCutOff)
{
	unsigned int i,j;
	double PresentationVal;
	RGBApixel PixelColor;

	SetSize(Width,Height);
	SetBitDepth(24);

	for(i=0;i<Width;i++)
	{
		for(j=0;j<Height;j++)
		{
			PresentationVal = *(double*)(((char*)FirstValPtr)+((i*Height+j)*Stride));
			PresentationVal = pow(PresentationVal,2.0);
			PresentationVal = log10(PresentationVal)*10;
			//Raise by 100 (for color coding)
			PresentationVal += AttenuationCutOff;
			//Truncate exceedingly low values. Seriously, we're talking 10 ORDERS OF MAGNITUDE here!
			if(PresentationVal < 0.0)
			{
				PresentationVal = 0.0;
			}
			//Normalize
			PresentationVal = PresentationVal/AttenuationCutOff;
			//Color code to heart's content
			if(PresentationVal < 0.5)
			{
				PixelColor.Red = ebmpBYTE(255.0*(1.0-2.0*PresentationVal));
				PixelColor.Green = ebmpBYTE(255.0*(2.0*PresentationVal));
				PixelColor.Blue = 0;
			}
			else
			{
				PixelColor.Red = 0;
				PixelColor.Green = ebmpBYTE(255.0*(2.0-2.0*PresentationVal));
				PixelColor.Blue = ebmpBYTE(255.0*(2.0*PresentationVal-1.0));
			}
			SetPixel(i,(Height-1)-j,PixelColor);
		}
	}
}

void EasyGraph::PlotArrayIstogram(double const * FirstValPtr, size_t Stride, unsigned int NumVals, double PixelYAxisDelta, float XAxisPos, unsigned int r, unsigned int g, unsigned int b, bool AddColors)
{
	int iNumVals = NumVals;
	int i,j;
	double ValueStor;
	int ColumnHeight,ColumnStart,ColumnEnd;

	RGBApixel Color, ExistingColor;
	Color.Red = r;
	Color.Green = g;
	Color.Blue = b;
	Color.Alpha = 0;

	if(iNumVals > TellWidth())
	{
		SetSize(iNumVals,TellHeight());
	}
	
	int XasisPosInPixels = int((1.0f-XAxisPos)*float(TellHeight()));

	for(i=0;i<iNumVals;i++)
	{
		ValueStor = *(double*)(((char*)FirstValPtr)+(i*Stride));
		ColumnHeight = int(floor(ValueStor/PixelYAxisDelta+0.5));

		if(ColumnHeight > 0)
		{
			ColumnStart = XasisPosInPixels - ColumnHeight;
			ColumnEnd = XasisPosInPixels;
		}
		else
		{
			ColumnStart = XasisPosInPixels;
			ColumnEnd = XasisPosInPixels - ColumnHeight;
		}
		if(ColumnStart < 0) {ColumnStart = 0;}
		if(ColumnEnd >= TellHeight()) {ColumnEnd = (TellHeight());}

		for(j=ColumnStart;j<ColumnEnd;j++)
		{
			if(AddColors == true)
			{
				ExistingColor = GetPixel(i,j);
				Color.Red = r + ExistingColor.Red;
				Color.Green = g + ExistingColor.Green;
				Color.Blue = b + ExistingColor.Blue;
				if(Color.Red>255) {Color.Red=255;}
				if(Color.Green>255) {Color.Green=255;}
				if(Color.Blue>255) {Color.Blue=255;}
			}
			SetPixel(i,j,Color);
		}
	}
}

void EasyGraph::PlotArrayIstogram(float const * FirstValPtr, size_t Stride, unsigned int NumVals, float PixelYAxisDelta, float XAxisPos, unsigned int r, unsigned int g,unsigned int b, bool AddColors)
{
	int iNumVals = NumVals;
	int i,j;
	float ValueStor;
	int ColumnHeight,ColumnStart,ColumnEnd;

	RGBApixel Color, ExistingColor;
	Color.Red = r;
	Color.Green = g;
	Color.Blue = b;
	Color.Alpha = 0;

	if(iNumVals > TellWidth())
	{
		SetSize(iNumVals,TellHeight());
	}
	
	int XasisPosInPixels = int((1.0f-XAxisPos)*float(TellHeight()));

	for(i=0;i<iNumVals;i++)
	{
		ValueStor = *(float*)(((char*)FirstValPtr)+(i*Stride));
		ColumnHeight = int(floor(ValueStor/PixelYAxisDelta+0.5));

		if(ColumnHeight > 0)
		{
			ColumnStart = XasisPosInPixels - ColumnHeight;
			ColumnEnd = XasisPosInPixels;
		}
		else
		{
			ColumnStart = XasisPosInPixels;
			ColumnEnd = XasisPosInPixels - ColumnHeight;
		}
		if(ColumnStart < 0) {ColumnStart = 0;}
		if(ColumnEnd >= TellHeight()) {ColumnEnd = (TellHeight());}

		for(j=ColumnStart;j<ColumnEnd;j++)
		{
			if(AddColors == true)
			{
				ExistingColor = GetPixel(i,j);
				Color.Red = r + ExistingColor.Red;
				Color.Green = g + ExistingColor.Green;
				Color.Blue = b + ExistingColor.Blue;
				if(Color.Red>255) {Color.Red=255;}
				if(Color.Green>255) {Color.Green=255;}
				if(Color.Blue>255) {Color.Blue=255;}
			}
			SetPixel(i,j,Color);
		}
	}
}
