#include "FFMpegWrapper.h"

#include "PCMWAV.h"

FFMpegWrapper::FFMpegWrapper(void):
m_pFormatCtx(NULL),
m_pCodecCtx(NULL),
m_pCodec(NULL),
m_iStreamIndex(-1)
{
	av_register_all();
}

FFMpegWrapper::~FFMpegWrapper(void)
{
}

bool FFMpegWrapper::OpenSource(LPSTR FileName)
{
	UINT i;

	//Try opening file
	if(av_open_input_file(&m_pFormatCtx, FileName, NULL, 0, NULL)!=0)
	{
		//Error Opening File;
		return false;
	}

	//File opened successfully;

	//Retrieve stream information
	if(av_find_stream_info(m_pFormatCtx)<0)
	{
		//Unable to find stream information;
		return false;
	}

	//cout << "The file holds " << pFormatCtx->nb_streams << " stream(s)\n";

	//Try finding first audio stream available
	for(i=0; i<m_pFormatCtx->nb_streams; i++)
	{
		if(m_pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			m_iStreamIndex = i;
			break;
		}
	}
	if(m_iStreamIndex == -1)
	{
		//Unable to find audio stream
		return false;
	}

	//Get a pointer to the codec context for the video stream
	m_pCodecCtx=m_pFormatCtx->streams[m_iStreamIndex]->codec;


	//Find the decoder for the audio stream
	m_pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id);
	if(m_pCodec==NULL) 
	{
		//Unsupported codec
		return false;
	}
	else
	{
		//try opening codec
		if(avcodec_open(m_pCodecCtx, m_pCodec)<0)
		{
			//Could not open codec
			return false;
		}
	}
	return true;
}

void FFMpegWrapper::CloseSource(void)
{
	av_close_input_file(m_pFormatCtx);
}
	
bool FFMpegWrapper::DecodeToWav(PCMWAV &Output)
{
	int InputBufSize = 0;
	uint8_t* InputBuffer = NULL;

	int SamplesBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
	int SamplesBufSizeCpy = SamplesBufSize;
	CACHE_ALIGN int16_t SamplesBuf[AVCODEC_MAX_AUDIO_FRAME_SIZE/2];

	unsigned int DecodedLen = 0;
	unsigned int TotalSamplesDecoded = 0;

	Output.SetNumChannels(m_pCodecCtx->channels);
	Output.SetSampleRate(m_pCodecCtx->sample_rate);
	//Guess a possible number of samples (aim to overshoot a little so that no further memory allocation and copies are triggered)
	//While decoding the actual number of samples will be counted and the array in the wavefile resized accordingly
	//Better use a bit more memory (and twice the mem during resizig) than keeping resizing and copying, as that really kills performance
	if(m_pFormatCtx->duration > 0)
	{
		Output.SetNumSamples((((unsigned int)m_pFormatCtx->duration/AV_TIME_BASE)+1)*Output.GetNumChannels()*Output.GetSampleRate());
	}
	//If duration is not specified (or is bogus specified, e.g. less than zero),
	//set duration to a resonable guess (60 secs).
	else
	{
		Output.SetNumSamples(60*Output.GetNumChannels()*Output.GetSampleRate());
	}

	while(av_read_frame(m_pFormatCtx, &sPacket)>=0) 
	{
		if(sPacket.stream_index == m_iStreamIndex) 
		{
			InputBuffer = (uint8_t*)av_malloc(sPacket.size+FF_INPUT_BUFFER_PADDING_SIZE);
			InputBufSize = sPacket.size;
			memset(InputBuffer,0x0,sizeof(InputBuffer));
			memcpy(InputBuffer,sPacket.data,sPacket.size);

			int BytesLeftToDecode = InputBufSize;
			uint8_t* StartDecodePtr = InputBuffer;
			while(BytesLeftToDecode>0)
			{
				SamplesBufSizeCpy = SamplesBufSize;
				DecodedLen = avcodec_decode_audio2(m_pCodecCtx, SamplesBuf, &SamplesBufSizeCpy, StartDecodePtr, BytesLeftToDecode);
				if(DecodedLen < 0)
				{
					//DECODING ERROR
					return false;
				}
				else
				{
					BytesLeftToDecode -= DecodedLen;
					StartDecodePtr += DecodedLen;
					if(SamplesBufSizeCpy>0)
					{
						TotalSamplesDecoded += SamplesBufSizeCpy/2;
						if(TotalSamplesDecoded > Output.GetNumSamples())
						{
							Output.SetNumSamples(2*TotalSamplesDecoded);
						}
						Output.RawCopy(TotalSamplesDecoded-(SamplesBufSizeCpy/2),SamplesBuf,SamplesBufSizeCpy/2);
					}
				}
			}

			av_free(InputBuffer);
		}
		av_free_packet(&sPacket);
	}

	//If we guessed the amount of samples wrong (likely), resize to correct length
	if(Output.GetNumSamples() != TotalSamplesDecoded) 
	{
		Output.SetNumSamples(TotalSamplesDecoded);
	}

	return true;
}
