#pragma once

#include <windows.h>

class PCMWAV;

extern "C" 
{
	#include "avcodec.h"
	#include "avformat.h"
}

#define CACHE_LINE  32
#define CACHE_ALIGN __declspec(align(CACHE_LINE))

class FFMpegWrapper
{
public:
	FFMpegWrapper(void);
	~FFMpegWrapper(void);
	bool OpenSource(LPSTR FileName);
	void CloseSource(void);
	bool DecodeToWav(PCMWAV &Output);

protected:
	FFMpegWrapper(const FFMpegWrapper&);

	AVFormatContext *m_pFormatCtx;
	AVCodecContext *m_pCodecCtx;
	AVCodec *m_pCodec;
	int m_iStreamIndex;
	AVPacket sPacket;
};
