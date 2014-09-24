#include <iostream>
#include <fstream>
#include <math.h>

#include "AudioLib.h"

#include "EasyGraph.h"
#include "EasyBMP\\EasyBMP.h"

using namespace std;

int main()
{
	EasyGraph Graph;
	
	unsigned int i,j,k;

	char FileName [1024];

	cout << "Input path (relative to program location) of input file\n";
	cin >> FileName;
	
	FFMpegWrapper FFMpegUtil;
	PCMWAV wavefile;
	if(FFMpegUtil.OpenSource(FileName)==false)
	{
		cout << "Error opening source file\n";
		return 1;
	}
	if(FFMpegUtil.DecodeToWav(wavefile)==false)
	{
		cout << "Error decoding source file\n";
		return 1;
	}
	FFMpegUtil.CloseSource();

	char InputStr [128];
	bool PerformFullAnalysis = false;
	cout << "\nInput g to generate graphs, anything else to only display track BPM\nWarning: generating graphs requires a significant amount of system memory\nand can take a while.\nDo not use the option if running the program from a non writeable location.\n\n";
	cin >> InputStr;
	cout << "\n";
	if(InputStr[0] == 'G' || InputStr[0] == 'g')
	{
		PerformFullAnalysis = true;
	}
	
	//Perform STFT and create feature vector
	AudioAnalyzer FourierUtil;
	FFTWOutput FourierOutput;
	AudioFeatures TrackFeatures;
	AudioSections TrackSections;
	FourierUtil.SetWindowLen(0.01f);
	FourierUtil.SetOverlap(0.5f);
	FourierUtil.SetWindowingFunction(AudioAnalyzer::HANN);
	FourierUtil.UsePadding(true);

	if(PerformFullAnalysis == true)
	{
		cout << "Generating full periodogram\n";
		FourierUtil.ComputeFFT(wavefile,0,FourierOutput);
		cout << "Generating feature vector\n";
		FourierUtil.ComputeAudioFeatures(FourierOutput,TrackFeatures);
	}
	else
	{
		cout << "Generating feature vector\n";
		FourierUtil.ComputeAudioFeatures(wavefile,0,TrackFeatures);
	}

	cout << "Determining beat locations and BPM\n\n";
	TrackSections.Generate(TrackFeatures,2000.0f);

	if(PerformFullAnalysis == true)
	{
		cout << "Generating visualizations:\n\n";
		
		cout << "Graphing periodogram (1/5)...\n";
		Graph.PlotDBSpectrogram(FourierOutput.Element(0,0),sizeof(fftw_complex),FourierOutput.NumWindows,FourierOutput.NumFreqBins,100.0);
		Graph.WriteToFile("Periodogram.bmp");
		cout << "Written to Periodogram.bmp\n";
		Graph.SetSize(TrackFeatures.GetNumWindows(),1000);

		cout << "Graphing power, centroid frequency and spectral spread over time (2/5)...\n";
		Graph.SetBGColor(0,0,0);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->Power,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),100000000.0,0.0f,255,0,0,true);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->FreqCentroid,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.001,0.0f,0,255,0,true);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->FreqSpread,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.00001,0.0f,0,0,255,true);
		Graph.WriteToFile("PCSGraph.bmp");
		cout << "Written to PCSGraph.bmp\n";

		cout << "Graphing rhythm tracks (3/5)...\n";
		Graph.SetBGColor(0,0,0);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->PowerDelta,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.001,0.5f,255,0,0,true);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->FreqCentroidDelta,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.001,0.5f,0,255,0,true);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->FreqSpreadDelta,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.001,0.5f,0,0,255,true);
		Graph.WriteToFile("RythmTracks.bmp");
		cout << "Written to RythmTracks.bmp\n";


		cout << "Graphing rhythm tracks interpolation (4/5)...\n";
		Graph.SetBGColor(255,255,255);
		Graph.PlotArrayIstogram(&TrackFeatures.GetFeaturesArrPtr()->RhythmInterpolation,sizeof(WindowFeatures),TrackFeatures.GetNumWindows(),0.001,0.0f,0,0,0,false);

		RGBApixel Color;
		Color.Red = 255;
		Color.Green = 0;
		Color.Blue = 0;
		for(i=0;i<TrackSections.GetNumSections();i++)
		{
			for(j=0;j<TrackSections.Section(i).BeatLocations.size();j++)
			{
				for(k=0;k<50;k++)
				{
					Graph.SetPixel(TrackSections.Section(i).BeatLocations[j],k,Color);
				}
			}
		}
		Graph.WriteToFile("RythmInterpolation.bmp");
		cout << "Written to RythmInterpolation.bmp\n";

		cout << "Graphing speed in BPM across sections (5/5)...\n";
		Graph.SetSize(TrackSections.GetNumSections(),600);
		Graph.SetBGColor(255,255,255);
		Graph.PlotArrayIstogram(&TrackSections.GetSectionsArrPtr()->BPM,sizeof(AudioSectionStr),TrackSections.GetNumSections(),1.0f,0.0f,0,0,0,false);
		Graph.WriteToFile("BPMGraph.bmp");
		cout << "Written to BPMGraph.bmp\n\n";
	}

	cout << "\nTrack BPM estimation: " << TrackSections.GetGlobalBPM() << "\n\n";
	
	cout << "Press any key to quit\n";
	cin.ignore();
	cin.get();
	return 0;
}