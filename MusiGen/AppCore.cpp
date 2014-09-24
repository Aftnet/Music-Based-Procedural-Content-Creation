#include "AppCore.h"

AppCore::AppCore(void):
m_pDSoundDevice(NULL),
m_pDSoundPrimaryBuffer(NULL),
m_uiBGMBufLenSamples(256*1024),  //MAKE SURE IT IS AN EVEN VALUE!!!!!!!!
m_pDSoundBGMBuffer(NULL),
m_pDSoundBGMBufferInt(NULL),
m_eGameState(IDLE_STATE),
m_PlayerShip(GameEntity::PLAYER_SHIP),
m_fPlayerFireDelayMs(100.0f),
m_fOffScreenTolerance(500.0f),
m_dPI(4.0*atan(1.0))
{
	UINT i;

	memset(m_sOpenFilePath,0x0,m_cuiOpenFilePathLen*sizeof(TCHAR));

	//Make sure the BGM buffer length in bytes is even.
	if((m_uiBGMBufLenSamples%2)!=0)
	{
		m_uiBGMBufLenSamples++;
	}

	for(i=0;i<256;i++)
	{
		m_bKeys[i] = false;
	}

	m_AudioAnalyzer.SetWindowLen(0.01f);
	m_AudioAnalyzer.SetOverlap(0.5f);
	m_AudioAnalyzer.SetWindowingFunction(AudioAnalyzer::HANN);
	m_AudioAnalyzer.UsePadding(true);
}

AppCore::~AppCore(void)
{
}

void AppCore::MainLoop(void)
{
	std::list<GameEntity>::iterator ListIt,ListIt2;
	if(m_eGameState == GAMEPLAY_STATE)
	{
		//Get Time Deltas
		m_fTimeSinceGameStart = m_GlobalTimer.GetElapsedMsecs();
		m_fFrameTimeDelta = m_FrameTimer.GetElapsedMsecs();
		m_FrameTimer.SetTimeBase();

		//Return to idle 3 seconds after track is finished
		if(m_fTimeSinceGameStart > (m_fBGMDurationMs + 3000))
		{
			SetToIdle();
		}

		//Refill BGM DirectSound buffer if needed
		RefreshBGMDSoundBuffer();

		//Spawn new enemies according to triggered game events
		if(m_GameEventsList.size() > 0)
		{
			if(m_fTimeSinceGameStart > m_GameEventsList.begin()->Time)
			{
				for(ListIt=m_GameEventsList.begin()->Entities.begin(); ListIt!=m_GameEventsList.begin()->Entities.end(); ListIt++)
				{
					m_EnemiesList.push_back(*ListIt);
				}
				m_GameEventsList.erase(m_GameEventsList.begin());
			}
		}

		//Respond to input
		m_PlayerShip.m_vVel = Vector4(0.0f,0.0f,0.0f,1.0f);
		if(m_bKeys['W'])
		{
			m_PlayerShip.m_vVel.y += 1.0f;
		}
		if(m_bKeys['S'])
		{
			m_PlayerShip.m_vVel.y -= 1.0f;
		}
		if(m_bKeys['A'])
		{
			m_PlayerShip.m_vVel.x -= 1.0f;
		}
		if(m_bKeys['D'])
		{
			m_PlayerShip.m_vVel.x += 1.0f;
		}
		if(m_bKeys[VK_SPACE])
		{
			if(m_PlayerLastShotTimer.HavePassed(m_fPlayerFireDelayMs))
			{
				m_PlayerLastShotTimer.SetTimeBase();
				GameEntity TempEnt(GameEntity::PLAYER_BULLET);
				TempEnt.m_vPos = m_PlayerShip.m_vPos + Vector4(0.0f,20.0f,0.0f,1.0f);
				TempEnt.m_vVel = 1.5f * Vector4(0.0f,1.0f,0.0f,1.0f);
				m_PlayerBulletsList.push_back(TempEnt);
			}
		}
		//Calculate player ship velocity
		m_PlayerShip.m_vVel.NormaliseSelf();
		m_PlayerShip.m_vVel = 0.5f*m_PlayerShip.m_vVel;

		//Have enemies firing
		UINT NumBulletsPerShot = 12;
		if(m_EnemyShip1LastShotTimer.GetElapsedMsecs() > 2000.0f)
		{
			m_EnemyShip1LastShotTimer.SetTimeBase();
			for(ListIt = m_EnemiesList.begin(); ListIt != m_EnemiesList.end(); ListIt++)
			{
				if(ListIt->GetType() == GameEntity::ENEMY_SHIP_01)
				{
					UINT i;
					GameEntity TempBullet(GameEntity::ENEMY_BULLET_01);

					for(i=0;i<NumBulletsPerShot;i++)
					{
						float alpha = float(i)/float(NumBulletsPerShot)*(2.0f*float(m_dPI));
						TempBullet.m_vPos = ListIt->m_vPos + Vector4(30.0f*sinf(alpha),30.0f*cosf(alpha),0.0f,1.0f);
						TempBullet.m_vVel = 1.2f*ListIt->m_vVel.Length()*Vector4(sinf(alpha),cosf(alpha),0.0f,1.0f);
						m_EnemyBulletsList.push_back(TempBullet);
					}
				}
			}
		}

		//Move Entities
		m_PlayerShip.m_vPos += m_fFrameTimeDelta*m_PlayerShip.m_vVel;
		for(ListIt = m_PlayerBulletsList.begin(); ListIt != m_PlayerBulletsList.end(); ListIt++)
		{
			ListIt->m_vPos += m_fFrameTimeDelta*ListIt->m_vVel;
		}
		for(ListIt = m_EnemiesList.begin(); ListIt != m_EnemiesList.end(); ListIt++)
		{
			ListIt->m_vPos += m_fFrameTimeDelta*ListIt->m_vVel;
		}
		for(ListIt = m_EnemyBulletsList.begin(); ListIt != m_EnemyBulletsList.end(); ListIt++)
		{
			ListIt->m_vPos += m_fFrameTimeDelta*ListIt->m_vVel;
		}

		//Ensure player ship is always visible
		if(m_PlayerShip.m_vPos.x < float(m_ppSpritesPtrArr[PLAYERSPR]->GetSize().right/2)) {m_PlayerShip.m_vPos.x = float(m_ppSpritesPtrArr[PLAYERSPR]->GetSize().right/2);}
		if(m_PlayerShip.m_vPos.x > float(m_ClientRec.right - (m_ppSpritesPtrArr[PLAYERSPR]->GetSize().right/2))) {m_PlayerShip.m_vPos.x = float(m_ClientRec.right - (m_ppSpritesPtrArr[PLAYERSPR]->GetSize().right/2));}
		if(m_PlayerShip.m_vPos.y < (m_ppSpritesPtrArr[PLAYERSPR]->GetSize().bottom/2)) {m_PlayerShip.m_vPos.y = float(m_ppSpritesPtrArr[PLAYERSPR]->GetSize().bottom/2);}
		if(m_PlayerShip.m_vPos.y > (m_ClientRec.bottom - (m_ppSpritesPtrArr[PLAYERSPR]->GetSize().bottom/2))) {m_PlayerShip.m_vPos.y = float(m_ClientRec.bottom - (m_ppSpritesPtrArr[PLAYERSPR]->GetSize().bottom/2));}
		
		//Check for collisions and respond
		//Player and enemy bullets
		ListIt = m_EnemyBulletsList.begin();
		while(ListIt != m_EnemyBulletsList.end())
		{
			if((ListIt->m_vPos-m_PlayerShip.m_vPos).Length() < (m_PlayerShip.GetRadius()+ListIt->GetRadius()))
			{
				//Hit occurred: remove enemy bullet (and do something else to the player?)
				ListIt = m_EnemyBulletsList.erase(ListIt);
			}
			else
			{
				ListIt++;
			}
		}
		//Player bullets and enemies
		bool AuxBool;
		ListIt = m_PlayerBulletsList.begin();
		while(ListIt != m_PlayerBulletsList.end())
		{
			AuxBool = true;
			ListIt2 = m_EnemiesList.begin();
			while(ListIt2 != m_EnemiesList.end())
			{
				if((ListIt->m_vPos-ListIt2->m_vPos).Length() < (ListIt->GetRadius()+ListIt2->GetRadius()))
				{
					ListIt = m_PlayerBulletsList.erase(ListIt);
					AuxBool = false;
					ListIt2->m_uiEnergy--;
					if(ListIt2->m_uiEnergy < 1)
					{
						m_EnemiesList.erase(ListIt2);
					}
					ListIt2 = m_EnemiesList.end();
				}
				else
				{
					ListIt2++;
				}
			}
			if(AuxBool == true)
			{
				ListIt++;
			}
		}
		//Player and enemies (do I need it?)

		//Remove entities too far off the screen
		ListIt = m_PlayerBulletsList.begin();
		while(ListIt != m_PlayerBulletsList.end())
		{
			if((ListIt->m_vPos.x < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.x > (float(m_ClientRec.right)+m_fOffScreenTolerance)) || (ListIt->m_vPos.y < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.y > (float(m_ClientRec.bottom)+m_fOffScreenTolerance)))
			{
				ListIt = m_PlayerBulletsList.erase(ListIt);
			}
			else
			{
				ListIt++;
			}
		}
		ListIt = m_EnemiesList.begin();
		while(ListIt != m_EnemiesList.end())
		{
			if((ListIt->m_vPos.x < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.x > (float(m_ClientRec.right)+m_fOffScreenTolerance)) || (ListIt->m_vPos.y < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.y > (float(m_ClientRec.bottom)+m_fOffScreenTolerance)))
			{
				ListIt = m_EnemiesList.erase(ListIt);
			}
			else
			{
				ListIt++;
			}
		}
		ListIt = m_EnemyBulletsList.begin();
		while(ListIt != m_EnemyBulletsList.end())
		{
			if((ListIt->m_vPos.x < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.x > (float(m_ClientRec.right)+m_fOffScreenTolerance)) || (ListIt->m_vPos.y < (0.0f - m_fOffScreenTolerance)) || (ListIt->m_vPos.y > (float(m_ClientRec.bottom)+m_fOffScreenTolerance)))
			{
				ListIt = m_EnemyBulletsList.erase(ListIt);
			}
			else
			{
				ListIt++;
			}
		}
	}
	DisplayFrame();
}

void AppCore::InitResources(HINSTANCE InstanceHandle, HWND WindowHandle)
{
	//record handles to program instance and program main window (used for almost everything)
	m_hinst = InstanceHandle;
	m_hmwnd = WindowHandle;
	//Get main window client area description
	GetClientRect(m_hmwnd,&m_ClientRec);
	//Acquire DC to main window client area (front buffer)
	m_hFrontBuffer = GetDC(m_hmwnd);
	//Create Back buffer to be compatible with front buffer
	m_hBackBuffer = CreateCompatibleDC(m_hFrontBuffer);
	//Create a temporary device contest to be used as a source for copying the various bitmaps to the back buffer
	m_hTempBuffer = CreateCompatibleDC(m_hBackBuffer);

	//Create bitmaps for Device Contexts
	m_hFrontBufferBitmap = CreateCompatibleBitmap(m_hFrontBuffer, m_ClientRec.right, m_ClientRec.bottom);
	m_hBackBufferBitmap = (HBITMAP)SelectObject(m_hBackBuffer, m_hFrontBufferBitmap);
	FillRect(m_hBackBuffer, &m_ClientRec, (HBRUSH)GetStockObject(0));

	//Create sprites
	m_ppSpritesPtrArr[PLAYERSPR] = new Sprite(IDB_PLAYER, IDB_PLAYERMASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[PLAYERBULLETSPR] = new Sprite(IDB_PLAYERBULLET, IDB_PLAYERBULLETMASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[ENEMY01SPR] = new Sprite(IDB_ENEMY01, IDB_ENEMY01MASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[ENEMY02SPR] = new Sprite(IDB_ENEMY02, IDB_ENEMY02MASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[ENEMY03SPR] = new Sprite(IDB_ENEMY03, IDB_ENEMY03MASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[ENEMYBULLET01SPR] = new Sprite(IDB_ENEMYBULLET01, IDB_ENEMYBULLET01MASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[IDLESPR] = new Sprite(IDB_IDLE, IDB_BGMASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[LOADING01SPR] = new Sprite(IDB_LOADING1, IDB_BGMASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[LOADING02SPR] = new Sprite(IDB_LOADING2, IDB_BGMASK, m_hBackBuffer, m_hTempBuffer);
	m_ppSpritesPtrArr[LOADING03SPR] = new Sprite(IDB_LOADING3, IDB_BGMASK, m_hBackBuffer, m_hTempBuffer);

	//DirectSound initialization
	//Device
	if(FAILED(DirectSoundCreate8(NULL,&m_pDSoundDevice,NULL)))
	{
		MessageBox(NULL, TEXT("Error Initializing DirectSound"), NULL, MB_ICONERROR | MB_OK);
	}
	//Set cooperation level to priority to be able to output in 16 bits per sample as opposed to just 8
	if(FAILED(m_pDSoundDevice->SetCooperativeLevel(m_hmwnd,DSSCL_PRIORITY)))
	{
		MessageBox(NULL, TEXT("Error Initializing DirectSound"), NULL, MB_ICONERROR | MB_OK);
	}
	//Get interface to primary buffer
	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.lpwfxFormat = NULL;
	dsbd.dwBufferBytes = 0;
	dsbd.dwReserved = 0;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
	
	if(FAILED(m_pDSoundDevice->CreateSoundBuffer(&dsbd,&m_pDSoundPrimaryBuffer,NULL)))
	{
		MessageBox(NULL, TEXT("Error creating primary buffer"), NULL, MB_OK);
	}

	//Set output format for primary buffer
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2;
	wfx.nBlockAlign = wfx.nChannels * ( wfx.wBitsPerSample / 8 );
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if(FAILED(m_pDSoundPrimaryBuffer->SetFormat(&wfx)))
	{
		MessageBox(NULL, TEXT("Error setting output format"), NULL, MB_OK);
	}

}

void AppCore::FreeResources(void)
{
	UINT i;
	//Release DirectSound Resources
	if(m_pDSoundBGMBufferInt != NULL) {m_pDSoundBGMBufferInt->Stop(); m_pDSoundBGMBufferInt->Release();}
	if(m_pDSoundBGMBuffer != NULL) {m_pDSoundBGMBuffer->Release();}
	if(m_pDSoundPrimaryBuffer != NULL) {m_pDSoundPrimaryBuffer->Release();}
	if(m_pDSoundDevice != NULL) {m_pDSoundDevice->Release();}

	//Delete sprites
	for(i=0;i<NUMSPRITES;i++)
	{
		delete m_ppSpritesPtrArr[i];
	}

	//No idea why
	SelectObject(m_hBackBuffer,m_hBackBufferBitmap);
	//Release DCs
	DeleteDC(m_hBackBuffer);
	DeleteDC(m_hTempBuffer);
	ReleaseDC(m_hmwnd,m_hFrontBuffer);
}

bool AppCore::LoadTrack(void)
{
	//Stop and release current BGM buffer
	if(m_pDSoundBGMBufferInt != NULL) 
	{
		m_pDSoundBGMBufferInt->Stop();
		m_pDSoundBGMBufferInt->Release();
		m_pDSoundBGMBufferInt = NULL;
	}
	if(m_pDSoundBGMBuffer != NULL) 
	{
		m_pDSoundBGMBuffer->Release();
		m_pDSoundBGMBuffer = NULL;
	}

	//If the user hasn't made a choice, return false to signal failure to open a track
	if (strlen(m_sOpenFilePath) == 0)
	{
		return false;
	}

	//Open file referenced by filename path string:

	//Set appropriate BG image for loading screen
	m_ppSpritesPtrArr[LOADING01SPR]->Draw(m_ClientRec.right/2,m_ClientRec.bottom/2);
	SwapBuffers();
	//Decode audio stream to PCM Wave
	if(m_FFMpegUtility.OpenSource(m_sOpenFilePath)==false)
	{
		MessageBox(NULL, TEXT("Error opening source file"), NULL, MB_OK);
		return false;
	}
	if(m_FFMpegUtility.DecodeToWav(m_PCMWave)==false)
	{
		MessageBox(NULL, TEXT("Error decoding source file"), NULL, MB_OK);
		return false;
	}
	//Close input file
	m_FFMpegUtility.CloseSource();
	//Zero file path string so that whether the user has aborted the operation can be detected
	memset(m_sOpenFilePath,0x0,m_cuiOpenFilePathLen*sizeof(TCHAR));
	
	//Create secondary DirectSound Buffer for BGM
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfx;

	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = DWORD(m_PCMWave.GetSampleRate());
	wfx.wBitsPerSample = 16;
	wfx.nChannels = m_PCMWave.GetNumChannels();
	wfx.nBlockAlign = wfx.nChannels * ( wfx.wBitsPerSample / 8 );
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
	dsbd.lpwfxFormat = &wfx;
	dsbd.dwBufferBytes = m_uiBGMBufLenSamples*sizeof(short);
	dsbd.dwReserved = 0;
	dsbd.guid3DAlgorithm = DS3DALG_DEFAULT;
	
	if(FAILED(m_pDSoundDevice->CreateSoundBuffer(&dsbd,&m_pDSoundBGMBuffer,NULL)))
	{
		MessageBox(NULL, TEXT("Error creating primary buffer"), NULL, MB_OK);
		return false;
	}
	if(FAILED(m_pDSoundBGMBuffer->QueryInterface(IID_IDirectSoundBuffer8,(LPVOID*)(&m_pDSoundBGMBufferInt))))
	{
		MessageBox(NULL, TEXT("Error getting interface for BGM buffer"), NULL, MB_OK);
		return false;
	}

	void* pvAudioPtr1;
	DWORD  dwAudioBytes1;
	void* pvAudioPtr2;
	DWORD dwAudioBytes2;
	HRESULT res;
	res = m_pDSoundBGMBufferInt->Lock(0,0,&pvAudioPtr1,&dwAudioBytes1,&pvAudioPtr2,&dwAudioBytes2,DSBLOCK_ENTIREBUFFER);
	if(FAILED(res))
	{
		if(res == DSERR_BUFFERLOST)
		//Try restoring and locking again
		{
			m_pDSoundBGMBufferInt->Restore();
			res = m_pDSoundBGMBufferInt->Lock(0,0,&pvAudioPtr1,&dwAudioBytes1,&pvAudioPtr2,&dwAudioBytes2,DSBLOCK_ENTIREBUFFER);
		}
		if(FAILED(res))
		{
			MessageBox(NULL, TEXT("Unable to lock buffer"), NULL, MB_OK);
			return false;
		}
	}

	if((dwAudioBytes1 != (m_uiBGMBufLenSamples*sizeof(short))) || (pvAudioPtr2 != NULL) || (dwAudioBytes2 != 0))
	{
		MessageBox(NULL, TEXT("Buffer inconsistent with request"), NULL, MB_OK);
		return false;
	}
	//Initialize buffer to all zeros (silence)
	memset(pvAudioPtr1,0x0,m_uiBGMBufLenSamples*sizeof(short));
	//Fill the whole buffer with the beginning of the BGM samples, if it's long enough
	if(m_PCMWave.GetNumSamples() > m_uiBGMBufLenSamples)
	{
		memcpy(pvAudioPtr1,(void*)m_PCMWave.GetDataPtr(),m_uiBGMBufLenSamples*sizeof(short));
	}
	else
	{
		memcpy(pvAudioPtr1,(void*)m_PCMWave.GetDataPtr(),m_PCMWave.GetNumSamples()*sizeof(short));
	}
	//Set the samples sent to buffer counter
	m_uiBGMSamplesSentToBuffer = m_uiBGMBufLenSamples;
	//Mark the lower half of buffer for refresh
	m_bNextToRefreshIsLowerHalfOfBGMBuffer = true;

	res = m_pDSoundBGMBufferInt->Unlock(pvAudioPtr1,dwAudioBytes1,pvAudioPtr2,dwAudioBytes2);
	if(FAILED(res))
	{
		MessageBox(NULL, TEXT("Unable to unlock buffer"), NULL, MB_OK);
		return false;
	}

	if(FAILED(m_pDSoundBGMBufferInt->SetCurrentPosition(0)))
	{
		MessageBox(NULL, TEXT("Unable to set play cursor"), NULL, MB_OK);
		return false;
	}

	//Compute audio features:

	//Set appropriate BG image for loading screen
	m_ppSpritesPtrArr[LOADING02SPR]->Draw(m_ClientRec.right/2,m_ClientRec.bottom/2);
	SwapBuffers();
	//Compute audio features
	m_AudioAnalyzer.ComputeAudioFeatures(m_PCMWave,0,m_AudioFeatures);



	//Compute audio sections:

	//Set appropriate BG image for loading screen
	m_ppSpritesPtrArr[LOADING03SPR]->Draw(m_ClientRec.right/2,m_ClientRec.bottom/2);
	SwapBuffers();
	//Comute audio sections
	m_AudioSections.Generate(m_AudioFeatures,2000.0f);
	CreateGameEvents();
	
	//Confirm a file has been loaded


	return true;
}

void AppCore::StartGame(void)
{
	if(m_eGameState != GAMEPLAY_STATE)
	{
		//Reset player to initial position
		m_PlayerShip.m_vPos = Vector4(512.0f,60.0f,0.0f,1.0f);
		//Start BGM playback
		if(FAILED(m_pDSoundBGMBufferInt->Play(0,0,DSBPLAY_LOOPING)))
		{
			MessageBox(NULL, TEXT("Unable to play"), NULL, MB_OK);
		}
		m_GlobalTimer.SetTimeBase();
		m_FrameTimer.SetTimeBase();
		m_fTimeSinceGameStart = 0.0f;
		m_fFrameTimeDelta = 0.0f;
		m_fBGMDurationMs = float(m_PCMWave.GetNumSamples())/float(m_PCMWave.GetNumChannels()*m_PCMWave.GetSampleRate());
		m_fBGMDurationMs = 1000.0f * m_fBGMDurationMs;
		m_PlayerBulletsList.clear();
		m_EnemiesList.clear();
		m_EnemyBulletsList.clear();
	}
	m_eGameState = GAMEPLAY_STATE;
}

void AppCore::SetToIdle(void)
{
	if(m_eGameState != IDLE_STATE)
	{
		m_PlayerBulletsList.clear();
		m_EnemiesList.clear();
		m_EnemyBulletsList.clear();

		if(m_pDSoundBGMBufferInt != NULL) 
		{
			m_pDSoundBGMBufferInt->Stop();
		}
	}
	m_eGameState = IDLE_STATE;
}

void AppCore::KeyDown(UINT Key)
{
	if(Key < 256)
	{
		m_bKeys[Key] = true;
	}
}

void AppCore::KeyUp(UINT Key)
{
	if(Key < 256)
	{
		m_bKeys[Key] = false;
	}
}

HWND AppCore::GetWindowHandle(void) const
{
	return m_hmwnd;
}

UINT AppCore::GetOpenFilePathLen(void) const
{
	return m_cuiOpenFilePathLen;
}

LPTSTR AppCore::GetOpenFilePathPtr(void)
{
	return m_sOpenFilePath;
}

void AppCore::CreateGameEvents(void)
{
	UINT i,j,k;
	GameEvent TempEvent;
	m_GameEventsList.clear();
	float Tempo = m_AudioSections.GetGlobalBPM();
	double EventsTimeDeltaMs = 1500.0f * (100.0f / Tempo);
	double AvgPower, AvgCentroidFreq, AvgSpread;
	UINT AWHalfSize = 25;

	AvgPower = AvgCentroidFreq = AvgSpread = 0.0f;
	i = 1;
	j = UINT((double(i)*EventsTimeDeltaMs)/m_AudioFeatures.GetWindowStrideInMsecs());
	while(j<(m_AudioFeatures.GetNumWindows()-UINT(4000.0/m_AudioFeatures.GetWindowStrideInMsecs())))
	{
		//Record event time
		TempEvent.Time = float(i)*float(EventsTimeDeltaMs);
		//Clear entity list
		TempEvent.Entities.clear();

		//Determine average values for power, centroid frequency and spread
		for(k=j-AWHalfSize;k<(j+AWHalfSize+1);k++)
		{
			AvgPower += m_AudioFeatures.Window(k).Power;
			AvgCentroidFreq += m_AudioFeatures.Window(k).FreqSpread;
			AvgSpread += m_AudioFeatures.Window(k).FreqSpread;
		}
		AvgPower = AvgPower/double((2*AWHalfSize)+1);
		AvgCentroidFreq = AvgCentroidFreq/double((2*AWHalfSize)+1);
		AvgSpread = AvgSpread/double((2*AWHalfSize)+1);

		//Use previously computed values to determine which enemies to spawn
		GameEntity Ent1(GameEntity::ENEMY_SHIP_01);
		GameEntity Ent2(GameEntity::ENEMY_SHIP_02);
		GameEntity Ent3(GameEntity::ENEMY_SHIP_03);
		
		//Fighters:
		UINT NumFighters = UINT(AvgSpread/0.0006);
		float FighterFormationCenter = 512.0f+64.0f*float(sin(1200.0f*AvgCentroidFreq));
		float FighterFormationSpread = 512.0f+256.0f*float(sin(1200.0f*AvgCentroidFreq));
		for(k=0;k<NumFighters;k++)
		{
			float Xpos;
			if(NumFighters > 1)
			{
				Xpos = FighterFormationCenter - (FighterFormationSpread/2.0f) + float(k)*FighterFormationSpread/float(NumFighters-1);
			}
			else
			{
				Xpos = FighterFormationCenter;
			}
			if(AvgCentroidFreq < 0.005) //spawn from top, going down
			{
				Ent2.m_vPos = Vector4(Xpos,850.0f,0.0f,1.0f);
				Ent2.m_vVel = Vector4(0.0f,-0.00002f*pow(Tempo,2.0f),0.0f,0.0f);
				TempEvent.Entities.push_back(Ent2);
			}
			else //spawn from bottom, going up
			{
				Ent3.m_vPos = Vector4(Xpos,-100.0f,0.0f,1.0f);
				Ent3.m_vVel = Vector4(0.0f,0.00002f*pow(Tempo,2.0f),0.0f,0.0f);
				TempEvent.Entities.push_back(Ent3);
			}
		}

		//Bombers
		UINT NumBombers = UINT(AvgPower/30000000000.0);
		srand(UINT(double(i)*EventsTimeDeltaMs));
		for(k=0;k<NumBombers;k++)
		{
			float alpha, beta;
			alpha = 2.0f*float(m_dPI)*(float(rand())/float(RAND_MAX));
			Ent1.m_vPos = Vector4(512.0f-(670.0f*sinf(alpha)),384.0f+(670.0f*cosf(alpha)),0.0f,1.0f);
			alpha = 2.0f*float(m_dPI)*(float(rand())/float(RAND_MAX));
			beta = 100.0f*(float(rand())/float(RAND_MAX));
			Vector4 vTarget(512.0f-(beta*sinf(alpha)),384.0f+(beta*cosf(alpha)),0.0f,1.0f);
			Ent1.m_vVel = (0.00002f*pow(Tempo,2.0f))*(vTarget-Ent1.m_vPos).Normalise();
			TempEvent.Entities.push_back(Ent1);
		}
		
		//Push event to list
		m_GameEventsList.push_back(TempEvent);
		i++;
		j = UINT((double(i)*EventsTimeDeltaMs)/m_AudioFeatures.GetWindowStrideInMsecs());
	}
}

void AppCore::RefreshBGMDSoundBuffer(void)
{
	bool ActionRequired = false;
	HRESULT Res;
	DWORD PlayCursorPos, WriteOffset;
	Res = m_pDSoundBGMBufferInt->GetCurrentPosition(&PlayCursorPos,NULL);

	//Check if the buffer needs to be refilled
	if(m_bNextToRefreshIsLowerHalfOfBGMBuffer == true)
	{
		if(PlayCursorPos >= ((m_uiBGMBufLenSamples/2)*sizeof(short)))
		{
			WriteOffset = 0;
			ActionRequired = true;
		}
	}
	else
	{
		if(PlayCursorPos < ((m_uiBGMBufLenSamples/2)*sizeof(short)))
		{
			WriteOffset = ((m_uiBGMBufLenSamples/2)*sizeof(short));
			ActionRequired = true;
		}
	}

	if(ActionRequired == true)
	{
		void* pvAudioPtr1;
		DWORD  dwAudioBytes1;
		void* pvAudioPtr2;
		DWORD dwAudioBytes2;
		Res = m_pDSoundBGMBufferInt->Lock(WriteOffset,((m_uiBGMBufLenSamples/2)*sizeof(short)),&pvAudioPtr1,&dwAudioBytes1,&pvAudioPtr2,&dwAudioBytes2,NULL);
		if(FAILED(Res))
		{
			if(Res == DSERR_BUFFERLOST)
			//Try restoring and locking again
			{
				m_pDSoundBGMBufferInt->Restore();
				Res = m_pDSoundBGMBufferInt->Lock(WriteOffset,((m_uiBGMBufLenSamples/2)*sizeof(short)),&pvAudioPtr1,&dwAudioBytes1,&pvAudioPtr2,&dwAudioBytes2,NULL);
			}
			if(FAILED(Res))
			{
				MessageBox(NULL, TEXT("Unable to lock buffer"), NULL, MB_OK);
			}
		}

		if((dwAudioBytes1 != ((m_uiBGMBufLenSamples/2)*sizeof(short))) || (pvAudioPtr2 != NULL) || (dwAudioBytes2 != 0))
		{
			MessageBox(NULL, TEXT("Buffer inconsistent with request"), NULL, MB_OK);
		}
		
		//Fill the relevant half of the buffer:
		//Initialize to all zeros (silence)
		memset(pvAudioPtr1,0x0,((m_uiBGMBufLenSamples/2)*sizeof(short)));
		//If the end of the original samples array hasn't been reached
		if(m_uiBGMSamplesSentToBuffer<m_PCMWave.GetNumSamples())
		{
			//If enough samples remain to be played in the samples array, copy another chunk half the size of the dsound buffer
			if((m_uiBGMSamplesSentToBuffer+(m_uiBGMBufLenSamples/2)) < m_PCMWave.GetNumSamples())
			{
				memcpy(pvAudioPtr1,(void*)((m_PCMWave.GetDataPtr())+m_uiBGMSamplesSentToBuffer),((m_uiBGMBufLenSamples/2)*sizeof(short)));
			}
			//Otherwise copy as many as there are left and set the remaining part of the half buffer to zero
			else
			{
				memcpy(pvAudioPtr1,(void*)((m_PCMWave.GetDataPtr())+m_uiBGMSamplesSentToBuffer),(m_PCMWave.GetNumSamples()-m_uiBGMSamplesSentToBuffer)*sizeof(short));
			}
		}

		//Update the samples sent to buffer counter
		m_uiBGMSamplesSentToBuffer += m_uiBGMBufLenSamples/2;
		//Mark the other half of buffer for refresh
		m_bNextToRefreshIsLowerHalfOfBGMBuffer = !m_bNextToRefreshIsLowerHalfOfBGMBuffer;

		Res = m_pDSoundBGMBufferInt->Unlock(pvAudioPtr1,dwAudioBytes1,pvAudioPtr2,dwAudioBytes2);
		if(FAILED(Res))
		{
			MessageBox(NULL, TEXT("Unable to unlock buffer"), NULL, MB_OK);
		}
	}
}

void AppCore::DisplayFrame(void)
{
	std::list<GameEntity>::iterator ListIt;
	switch(m_eGameState)
	{
	case IDLE_STATE:
		m_ppSpritesPtrArr[IDLESPR]->Draw(m_ClientRec.right/2,m_ClientRec.bottom/2);
		break;

	case GAMEPLAY_STATE:
		for(ListIt = m_EnemyBulletsList.begin(); ListIt != m_EnemyBulletsList.end(); ListIt++)
		{
			m_ppSpritesPtrArr[ENEMYBULLET01SPR]->Draw(UINT(floor(ListIt->m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-ListIt->m_vPos.y)+0.5f)));
		}
		for(ListIt = m_PlayerBulletsList.begin(); ListIt != m_PlayerBulletsList.end(); ListIt++)
		{
			m_ppSpritesPtrArr[PLAYERBULLETSPR]->Draw(UINT(floor(ListIt->m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-ListIt->m_vPos.y)+0.5f)));
		}
		for(ListIt = m_EnemiesList.begin(); ListIt != m_EnemiesList.end(); ListIt++)
		{
			switch(ListIt->GetType())
			{
			case GameEntity::ENEMY_SHIP_01:
				m_ppSpritesPtrArr[ENEMY01SPR]->Draw(UINT(floor(ListIt->m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-ListIt->m_vPos.y)+0.5f)));
				break;
			case GameEntity::ENEMY_SHIP_02:
				m_ppSpritesPtrArr[ENEMY02SPR]->Draw(UINT(floor(ListIt->m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-ListIt->m_vPos.y)+0.5f)));
				break;
			case GameEntity::ENEMY_SHIP_03:
				m_ppSpritesPtrArr[ENEMY03SPR]->Draw(UINT(floor(ListIt->m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-ListIt->m_vPos.y)+0.5f)));
				break;
			}
			
		}
		m_ppSpritesPtrArr[PLAYERSPR]->Draw(UINT(floor(m_PlayerShip.m_vPos.x+0.5f)),UINT(floor((m_ClientRec.bottom-m_PlayerShip.m_vPos.y)+0.5f)));
		break;
	}

	SwapBuffers();
}

void AppCore::SwapBuffers(void)
{
	BitBlt(m_hFrontBuffer, m_ClientRec.left, m_ClientRec.top, m_ClientRec.right, m_ClientRec.bottom, m_hBackBuffer, 0, 0, SRCCOPY);
	FillRect(m_hBackBuffer, &m_ClientRec, (HBRUSH)GetStockObject(0));	
}