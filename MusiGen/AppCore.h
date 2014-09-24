#pragma once

#include <windows.h>
#include <vector>
#include <list>
#include <dsound.h>
#include "AudioLib.h"
#include "Sprite.h"
#include "Timer.h"
#include "GameEntities.h"

class AppCore
{
public:
	AppCore(void);
	~AppCore(void);

	void MainLoop(void);
	void InitResources(HINSTANCE InstanceHandle, HWND WindowHandle);
	void FreeResources(void);
	bool LoadTrack(void);
	void StartGame(void);
	void SetToIdle(void);
	void KeyDown(UINT Key);
	void KeyUp(UINT Key);

	HWND GetWindowHandle(void) const;
	UINT GetOpenFilePathLen(void) const;
	LPTSTR GetOpenFilePathPtr(void);

protected:
	AppCore(AppCore const&);

	void CreateGameEvents(void);
	void RefreshBGMDSoundBuffer(void);
	void DisplayFrame(void);
	void SwapBuffers(void);

	//Game data
	enum GameStatesEnum {IDLE_STATE, GAMEPLAY_STATE};
	GameStatesEnum m_eGameState;
	const float m_fPlayerFireDelayMs;
	const float m_fOffScreenTolerance;

	Timer m_GlobalTimer, m_FrameTimer, m_PlayerLastShotTimer, m_EnemyShip1LastShotTimer;
	float m_fTimeSinceGameStart, m_fFrameTimeDelta, m_fBGMDurationMs;

	GameEntity m_PlayerShip;
	std::list<GameEntity> m_PlayerBulletsList;
	std::list<GameEntity> m_EnemiesList;
	std::list<GameEntity> m_EnemyBulletsList;

	struct GameEvent
	{
		float Time;
		std::list<GameEntity> Entities;
	};
	std::list<GameEvent> m_GameEventsList;

	//Game sprites
	enum SpritesEnum {PLAYERSPR, PLAYERBULLETSPR, ENEMY01SPR, ENEMY02SPR, ENEMY03SPR, ENEMYBULLET01SPR, IDLESPR, LOADING01SPR, LOADING02SPR, LOADING03SPR, NUMSPRITES};
	Sprite *m_ppSpritesPtrArr[NUMSPRITES];
	
	//Windows resources
	HINSTANCE m_hinst;
	HWND m_hmwnd;
	RECT m_ClientRec;
	HDC m_hFrontBuffer, m_hBackBuffer, m_hTempBuffer;
	HBITMAP m_hFrontBufferBitmap, m_hBackBufferBitmap;
	bool m_bKeys[256];

	//DirectSound resources
	LPDIRECTSOUND8 m_pDSoundDevice;
	LPDIRECTSOUNDBUFFER m_pDSoundPrimaryBuffer;
	DWORD m_uiBGMBufLenSamples, m_uiBGMSamplesSentToBuffer;
	bool m_bNextToRefreshIsLowerHalfOfBGMBuffer;
	LPDIRECTSOUNDBUFFER m_pDSoundBGMBuffer;
	LPDIRECTSOUNDBUFFER8 m_pDSoundBGMBufferInt;

	//Audio track data
	static const UINT m_cuiOpenFilePathLen = 512;
	TCHAR m_sOpenFilePath[m_cuiOpenFilePathLen];
	PCMWAV m_PCMWave;
	FFMpegWrapper m_FFMpegUtility;
	AudioAnalyzer m_AudioAnalyzer;
	AudioFeatures m_AudioFeatures;
	AudioSections m_AudioSections;

	//Math Constants
	const double m_dPI;
};
