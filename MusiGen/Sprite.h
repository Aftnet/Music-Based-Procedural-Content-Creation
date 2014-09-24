#pragma once

#include <windows.h>
#include "resource.h"

class Sprite
{
public:
	//Create from file on disk
	Sprite(LPTSTR ImageFileName, LPTSTR MaskFileName, HDC DestDC, HDC TempDC);
	//Create from resource
	Sprite(int ImageResID, int MaskResID, HDC DestDC, HDC TempDC);
	RECT GetSize(void) const;
	void Draw(UINT Xpos, UINT Ypos);
	~Sprite(void);

protected:
	//handles to DCs
	HDC m_hDestBuffer, m_hTempBuffer;
	//handles to bitmaps
	HBITMAP m_hImg, m_hMask;
	//bitmap info structures
	BITMAP m_ImgInfo, m_Maskinfo;
};
