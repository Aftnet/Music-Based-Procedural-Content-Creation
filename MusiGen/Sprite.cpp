#include "Sprite.h"

Sprite::Sprite(LPTSTR ImageFileName, LPTSTR MaskFileName, HDC DestDC, HDC TempDC)
{
	m_hImg = NULL;
	m_hMask = NULL;

	m_hDestBuffer = DestDC;
	m_hTempBuffer = TempDC;

	m_hImg = (HBITMAP)LoadImage(NULL, ImageFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if(m_hImg == NULL)
	{
		MessageBox(NULL, ImageFileName, NULL, MB_ICONERROR | MB_OK);
	}
	GetObject(m_hImg, sizeof(m_ImgInfo), &m_ImgInfo);
	
	m_hMask = (HBITMAP)LoadImage(NULL, MaskFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if(m_hMask == NULL)
	{
		MessageBox(NULL, MaskFileName, NULL, MB_ICONERROR | MB_OK);
	}
	GetObject(m_hMask, sizeof(m_Maskinfo), &m_Maskinfo);
}

Sprite::Sprite(int ImageResID, int MaskResID, HDC DestDC, HDC TempDC)
{
	m_hImg = NULL;
	m_hMask = NULL;

	m_hDestBuffer = DestDC;
	m_hTempBuffer = TempDC;

	m_hImg = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(ImageResID));
	if(m_hImg == NULL)
	{
		MessageBox(NULL, TEXT("Unable to load image resource"), NULL, MB_ICONERROR | MB_OK);
	}
	GetObject(m_hImg, sizeof(m_ImgInfo), &m_ImgInfo);

	m_hMask = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(MaskResID));
	if(m_hMask == NULL)
	{
		MessageBox(NULL, TEXT("Unable to load mask resource"), NULL, MB_ICONERROR | MB_OK);
	}
	GetObject(m_hMask, sizeof(m_Maskinfo), &m_Maskinfo);
}

Sprite::~Sprite(void)
{
	DeleteObject(m_hMask);
	DeleteObject(m_hImg);
}

RECT Sprite::GetSize(void) const
{
	RECT TempRect;
	TempRect.top = 0;
	TempRect.left = 0;
	TempRect.right = m_ImgInfo.bmWidth;
	TempRect.bottom = m_ImgInfo.bmHeight;
	return TempRect;
}

void Sprite::Draw(UINT Xpos, UINT Ypos)
{
	BOOL SuccessFlag;
	HBITMAP originalBitMap = NULL;
	originalBitMap = (HBITMAP)SelectObject(m_hTempBuffer,m_hMask);
	SuccessFlag = BitBlt(m_hDestBuffer,(Xpos-(m_ImgInfo.bmWidth/2)),(Ypos-(m_ImgInfo.bmHeight/2)),m_ImgInfo.bmWidth,m_ImgInfo.bmHeight,m_hTempBuffer,0,0,SRCAND);
	originalBitMap = (HBITMAP)SelectObject(m_hTempBuffer,m_hImg);
	SuccessFlag = BitBlt(m_hDestBuffer,(Xpos-(m_ImgInfo.bmWidth/2)),(Ypos-(m_ImgInfo.bmHeight/2)),m_ImgInfo.bmWidth,m_ImgInfo.bmHeight,m_hTempBuffer,0,0,SRCPAINT);
}