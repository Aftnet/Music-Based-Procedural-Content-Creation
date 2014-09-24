#include <windows.h>
#include "resource.h"
#include "AppCore.h"

//Global Application Core instance
AppCore gAppCore;

//Function Declarations
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitializeOpenFileDialogStruc(OPENFILENAME& Input);

//Winmain, entry point
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
	//Create window class
	WNDCLASSEX  wcex;							
	//Specify window class
	wcex.cbSize        = sizeof (wcex);				
	wcex.style         = CS_HREDRAW | CS_VREDRAW;		
	wcex.lpfnWndProc   = WndProc;						
	wcex.cbClsExtra    = 0;								
	wcex.cbWndExtra    = 0;								
	wcex.hInstance     = hInstance;						
	wcex.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);														
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);							
	wcex.lpszClassName = TEXT("WinClass");
	wcex.hIconSm       = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	//Register window class
	RegisterClassEx (&wcex);		
	//Create main window
	HWND hwnd = CreateWindow(TEXT("WinClass"), TEXT("MusiGen"), WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, (1024+6), (768+44), NULL, NULL, hInstance, NULL);								
	if (!hwnd)
	{
		MessageBox(NULL, TEXT("Unable to create window"), NULL, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	//Get Application Core up and running
	gAppCore.InitResources(hInstance, hwnd);

	//Display main window
	ShowWindow (hwnd, nCmdShow);						
	UpdateWindow (hwnd);

	MSG msg;

	while (TRUE)					
	{							
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
			{
				break;
			}
			TranslateMessage (&msg);							
			DispatchMessage (&msg);
		}
		else
		{
			gAppCore.MainLoop();
		}
	}
	
	gAppCore.FreeResources();
	return int(msg.wParam);										
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)											
	{
	
	case WM_ENTERMENULOOP:
		gAppCore.SetToIdle();
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_FILE_OPENFILE:
			OPENFILENAME OpenFileStr;
			InitializeOpenFileDialogStruc(OpenFileStr);
			GetOpenFileName(&OpenFileStr);
			if(gAppCore.LoadTrack() == true)
			{
				gAppCore.StartGame();
			}
			break;
		case ID_FILE_EXIT:
			PostQuitMessage(0);
			break;
		}

	case WM_KEYDOWN:
		gAppCore.KeyDown(UINT(wParam));
		break;

	case WM_KEYUP:
		gAppCore.KeyUp(UINT(wParam));
		break;

	case WM_DESTROY:
		PostQuitMessage(0);					
		break;	

	/*case WM_CREATE:	
		break;

	case WM_MOVE:
		//AppCore::GetInstance().MainLoop();
		break;

	case WM_MOVING:
		//AppCore::GetInstance().MainLoop();
		break;	*/

	/*case WM_PAINT:
		//AppCore::GetInstance().MainLoop();
		break;*/			
	}													

	return DefWindowProc (hwnd, message, wParam, lParam);	
}

void InitializeOpenFileDialogStruc(OPENFILENAME& Input)
{
	// Initialize OPENFILENAME
	ZeroMemory(&Input, sizeof(Input));
	Input.lStructSize = sizeof(Input);
	Input.hwndOwner = gAppCore.GetWindowHandle();
	Input.lpstrFile = gAppCore.GetOpenFilePathPtr();
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	// ofn.lpstrFile[0] = '\0';
	Input.nMaxFile = gAppCore.GetOpenFilePathLen();
	Input.lpstrFilter = TEXT("Supported Formats\0*.wav;*.flac;*.mp3;*.aac;*.ogg;*.ogm;*.mp4;*.m4a;*.m4v;*.mkv\0\0");
	Input.nFilterIndex = 1;
	Input.lpstrFileTitle = NULL;
	Input.nMaxFileTitle = 0;
	Input.lpstrInitialDir = NULL;
	Input.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}