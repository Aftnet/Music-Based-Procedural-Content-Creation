#include "PCMWAV.h"

#include <fstream>

PCMWAV::PCMWAV(void)
{
	m_uiNumSamples = 0;

	m_iSize = 36;

	m_siNumChannels = 1;
	m_iSampleRate = 0;
	m_iByteRate = 0;
	m_siBlockAlign = 0;

	m_iSubchunk2Size = 0;
	m_pData = NULL;
}

PCMWAV::~PCMWAV(void)
{
	if(m_pData!=NULL)
	{
		delete[] m_pData;
	}
	m_pData = NULL;
}

unsigned int PCMWAV::GetNumChannels(void) const
{
	return m_siNumChannels;
}

void PCMWAV::SetNumChannels(unsigned int Val)
{
	m_siNumChannels = Val;

	ClearSamples();
}
	
unsigned int PCMWAV::GetSampleRate(void) const
{
	return m_iSampleRate;
}
	
void PCMWAV::SetSampleRate(unsigned int Val)
{
	m_iSampleRate = Val;

	ComputeParams();
}

unsigned int PCMWAV::GetNumSamples(void) const
{
	return m_uiNumSamples;
}

bool PCMWAV::SetNumSamples(unsigned int Val)
{
	if(Val == 0)
	{
		ClearSamples();
		return true;
	}
	if(m_siNumChannels > 0 && (Val%m_siNumChannels) == 0)
	{
		if(m_uiNumSamples != Val)
		{	
			short* TempPtr = new short[Val];
			if(Val > m_uiNumSamples)
			{
				memcpy(TempPtr,m_pData,m_uiNumSamples*2);
			}
			else
			{
				memcpy(TempPtr,m_pData,Val*2);
			}
			delete[] m_pData;
			m_pData = TempPtr;
			m_uiNumSamples = Val;
			ComputeParams();
		}
		return true;
	}
	else
	{
		return false;
	}
}

short PCMWAV::GetSample(unsigned int SampleID) const
{
	if(SampleID < m_uiNumSamples)
	{
		return m_pData[SampleID];
	}
	else
	{
		return m_pData[m_uiNumSamples-1];
	}
}

short PCMWAV::GetSample(unsigned short ChannelID, unsigned int SampleID) const
{
	unsigned int Ch, Sam;

	if(ChannelID < m_siNumChannels) {Ch = ChannelID;} else {Ch = m_siNumChannels-1;}
	if(SampleID < (m_uiNumSamples/m_siNumChannels)) {Sam = SampleID;} else {Sam = (m_uiNumSamples/m_siNumChannels)-1;}

	return m_pData[(Sam*m_siNumChannels)+Ch];
}

bool PCMWAV::SetSample(unsigned int SampleID, short Val)
{
	if(SampleID < m_uiNumSamples)
	{
		m_pData[SampleID] = Val;
		return true;
	}
	else
	{
		return false;
	}
}

bool PCMWAV::SetSample(unsigned short ChannelID, unsigned int SampleID, short Val)
{
	if((ChannelID < m_siNumChannels) && (SampleID < (m_uiNumSamples/m_siNumChannels)))
	{
		m_pData[(SampleID*m_siNumChannels)+ChannelID] = Val;
		return true;
	}
	else
	{
		return false;
	}
}

short * const PCMWAV::GetDataPtr(void)
{
	return m_pData;
}

bool PCMWAV::RawCopy(unsigned int SamplesOffset, void* SourcePtr, unsigned int SizeInSamples)
{
	if((SamplesOffset + SizeInSamples) <= (m_uiNumSamples))
	{
		memcpy((void*)(m_pData + SamplesOffset),SourcePtr,SizeInSamples*2);
		return true;
	}
	else
	{
		return false;
	}
}
bool PCMWAV::ReadFrom(LPTSTR FileName)
{
	//Do some error checking in the filename
	#ifdef UNICODE 
	std::wstring FileNameStr(FileName);
	#else
	std::string FileNameStr(FileName);
	#endif

	int DotPos = FileNameStr.find_last_of(TEXT("."));
	#ifdef UNICODE
	std::wstring ExtStr(FileNameStr.c_str()+(DotPos+1));
	#else
	std::string ExtStr(FileNameStr.c_str()+(DotPos+1));
	#endif

	//If extension is not wav in any combination of up and lowercase abort opening of the file
	if(!(((ExtStr[0] == TEXT('W')) || (ExtStr[0] == TEXT('w'))) && ((ExtStr[1] == TEXT('A')) || (ExtStr[1] == TEXT('a'))) && ((ExtStr[2] == TEXT('V')) || (ExtStr[2] == TEXT('v'))) && (ExtStr[3] == TEXT('\0'))))
	{
		return false;
	}
	
	//Open file and check for errors
	std::fstream File(FileName, std::fstream::in | std::fstream::binary);
	if(!File.is_open())
	{
		return false;
	}

	//Read headers from file and test for conformity
	char TempBuf[5] = "null";
	std::string TempStr;
	
	int TempInt;
	short TempShort;

	int FileSize, FileSampleRate, FileByteRate;
	short FileNumChannels, FileBlockAlign;

	//ChunkID, has to be == to "RIFF"
	File.read(TempBuf,4);
	TempStr.assign(TempBuf);
	if(TempStr.compare("RIFF")!=0)
	{
		return false;
	}
	//ChunkSize
	File.read((char*)&FileSize,4);
	//Format, has to be == "WAVE"
	File.read(TempBuf,4);
	TempStr.assign(TempBuf);
	if(TempStr.compare("WAVE")!=0)
	{
		return false;
	}

	//Subchunk1ID, has to be == to "fmt " (with a space)    
	File.read(TempBuf,4);
	TempStr.assign(TempBuf);
	if(TempStr.compare("fmt ")!=0)
	{
		return false;
	}
	//Subchunk1Size, has to be == to 16
	File.read((char*)&TempInt,sizeof(TempInt));
	if(TempInt!=16)
	{
		return false;
	}
	//AudioFormat, has to be == to 1
	File.read((char*)&TempShort,sizeof(TempShort));
	if(TempShort!=1)
	{
		return false;
	}
	//NumChannels, SampleRate, ByteRate and BlockAlign: can have any value
	File.read((char*)&FileNumChannels,sizeof(FileNumChannels));
	File.read((char*)&FileSampleRate,sizeof(FileSampleRate));
	File.read((char*)&FileByteRate,sizeof(FileByteRate));
	File.read((char*)&FileBlockAlign,sizeof(FileBlockAlign));
	//BitsPerSample, has to be == to 16
	File.read((char*)&TempShort,sizeof(TempShort));
	if(TempShort!=16)
	{
		return false;
	}

	File.read(TempBuf,4);
	TempStr.assign(TempBuf);
	if(TempStr.compare("data")!=0)
	{
		return false;
	}

	//If execution reaches this point the input wave file is valid and the class data structures are updated

	//Clear existing information
	ClearSamples();

	m_iSize = FileSize;

	m_siNumChannels = FileNumChannels;
	m_iSampleRate = FileSampleRate;
	m_iByteRate = FileByteRate;
	m_siBlockAlign = FileBlockAlign;
	
	File.read((char*)&m_iSubchunk2Size,sizeof(m_iSubchunk2Size));
	m_uiNumSamples = m_iSubchunk2Size/2;
	m_pData = new short[m_uiNumSamples];
	File.read((char*)m_pData,m_iSubchunk2Size);

	//Close file
	File.close();
	return true;
}

bool PCMWAV::SaveTo(LPTSTR FileName)
{
	std::string TempStr;

	//Open file and check for errors
	std::fstream File(FileName, std::fstream::out | std::fstream::binary);
	if(!File.is_open())
	{
		return false;
	}

	
	//Write to file
	int TempInt;
	short TempShort;

	TempStr.assign("RIFF");
	File.write(TempStr.c_str(),4);
	File.write((char*)&m_iSize,4);
	TempStr.assign("WAVE");
	File.write(TempStr.c_str(),4);

	TempStr.assign("fmt ");
	File.write(TempStr.c_str(),4);
	TempInt = 16;
	File.write((char*)&TempInt,sizeof(TempInt));
	TempShort = 1;
	File.write((char*)&TempShort,sizeof(TempShort));
	File.write((char*)&m_siNumChannels,sizeof(m_siNumChannels));
	File.write((char*)&m_iSampleRate,sizeof(m_iSampleRate));
	File.write((char*)&m_iByteRate,sizeof(m_iByteRate));
	File.write((char*)&m_siBlockAlign,sizeof(m_siBlockAlign));
	TempShort = 16;
	File.write((char*)&TempShort,sizeof(TempShort));

	TempStr.assign("data");
	File.write(TempStr.c_str(),4);
	File.write((char*)&m_iSubchunk2Size,sizeof(m_iSubchunk2Size));
	File.write((char*)m_pData,m_uiNumSamples*sizeof(short));

	//Close file
	File.close();
	return true;
}

void PCMWAV::ComputeParams(void)
{
	m_iByteRate = 2 * m_iSampleRate * m_siNumChannels;
	m_siBlockAlign = 2 * m_siNumChannels;

	m_iSubchunk2Size = 2 * m_uiNumSamples;
	m_iSize = 36 + m_iSubchunk2Size;
}

void PCMWAV::ClearSamples(void)
{
	m_uiNumSamples = 0;

	if(m_pData!=NULL)
	{
		delete[] m_pData;
	}
	m_pData = NULL;

	ComputeParams();
}
