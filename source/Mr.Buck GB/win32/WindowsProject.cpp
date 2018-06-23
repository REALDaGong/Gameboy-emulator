#include "stdafx.h"
#include "WindowsProject.h"
#pragma comment(lib,"Winmm.lib")
#include "commdlg.h"
#include <MMSystem.h>
#include "..\gbcore\GB_Main.h"
#define MAX_LOADSTRING 100
const int WINDOWX = 160;
const int WINDOWY = 144;
const int bit = 24;
const int TIMER_ID = 1;
int Frame = 0;
// 
HINSTANCE hInst;                                // 
WCHAR szTitle[MAX_LOADSTRING];                  // 
WCHAR szWindowClass[MAX_LOADSTRING];            // 
int TimerID;
static HDC screen_hdc;
static HWND screen_hwnd;
static HDC hCompatibleDC; //compatible HDC  
static HBITMAP hCompatibleBitmap; //BITMAP  
static HBITMAP hOldBitmap; //BITMAP                   
static BITMAPINFO binfo; //BITMAPINFO struct (header)  
static BYTE FrameBuffer[WINDOWX*WINDOWY*3];
static BOOL RomLoaded = 0;


void         OpenFile(HWND hwnd);

BOOL					PaintNewFrame(HWND hWnd,HDC &hdc);
void					Translate();
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    
    // init
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	
    // init window
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;


    }
	
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT3));

    MSG msg;

	//TimerID = timeSetEvent(16,1,RunANewFrame(),NULL,TIME_PERIODIC);
	
	
	
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	return (int) msg.wParam;
}




ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 160, 144, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE: 
	{
		UINT_PTR ret = SetTimer(hWnd, TIMER_ID,8,NULL);

		ZeroMemory(&binfo, sizeof(BITMAPINFO));
		binfo.bmiHeader.biBitCount = 24;      //24(RGB)32(RGBA)  
		binfo.bmiHeader.biCompression = BI_RGB;
		binfo.bmiHeader.biHeight = -144;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSizeImage = 0;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biWidth = 160;

		ZeroMemory(FrameBuffer, sizeof(FrameBuffer));
		 
		screen_hwnd = hWnd;
		screen_hdc = GetDC(screen_hwnd);

		  
		hCompatibleDC = CreateCompatibleDC(screen_hdc);
		hCompatibleBitmap = CreateCompatibleBitmap(screen_hdc, WINDOWX, WINDOWY);
		hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_OPEN:
				OpenFile(hWnd);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
           

			PaintNewFrame(hWnd,hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		KillTimer(hWnd, TIMER_ID);
		DeleteDC(screen_hdc);
		DeleteDC(hCompatibleDC);

        PostQuitMessage(0);
        break;
	case WM_TIMER:
		
		if (RomLoaded) {
			RunANewFrame();
			//loadKeyPress();
			PostMessage(hWnd, WM_PAINT, 0, 0);
		}
		
		
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_KEYDOWN:
	{using namespace GBCore;
	memory._memoryMapio[0] |= 0x10;
		switch (wParam) {
			
		case VK_LEFT:
			memory._KeyRow[1] &= 0xD;
			break;
		case VK_RIGHT:
			memory._KeyRow[1] &= 0xE;
			break;
		case VK_UP:
			memory._KeyRow[1] &= 0xB;
			break;
		case VK_DOWN:
			memory._KeyRow[1] &= 0x7;
			break;
		case 0x5a://Z
			memory._KeyRow[0] &= 0xE;
			break;
		case 0x58://X
			memory._KeyRow[0] &= 0xD;
			break;
		case VK_RETURN:
			memory._KeyRow[0] &= 0x7;
			break;
		case VK_SPACE:
			memory._KeyRow[0] &= 0xB;
			break;
		}
	}
		break;
	case WM_KEYUP:
	{using namespace GBCore;
	memory._memoryMapio[0] |= 0x10;
		switch (wParam) {
		case VK_LEFT:
			memory._KeyRow[1] |= 0x2;
			break;
		case VK_RIGHT:
			memory._KeyRow[1] |= 0x1;
			break;
		case VK_UP:
			memory._KeyRow[1] |= 0x4;
			break;
		case VK_DOWN:
			memory._KeyRow[1] |= 0x8;
			break;
		case 0x5a://Z
			memory._KeyRow[0] |= 0x1;
			break;
		case 0x58://X
			memory._KeyRow[0] |= 0x2;
			break;
		case VK_RETURN:
			memory._KeyRow[0] |= 0x8;
			break;
		case VK_SPACE:
			memory._KeyRow[0] |= 0x4;
			break;
		}
	}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}




BOOL PaintNewFrame(HWND hWnd,HDC &hdc) {
	Translate();
	RECT rect;
	GetClientRect(hWnd, &rect);
	SetStretchBltMode(screen_hdc, HALFTONE);
	int i=StretchDIBits(screen_hdc,0,0,rect.right-rect.left,rect.bottom-rect.top,0,0,WINDOWX,WINDOWY,FrameBuffer,&binfo, DIB_RGB_COLORS, SRCCOPY);
	return 1;
}

void Translate() {
	int color = 0;
	memset(FrameBuffer, 0, sizeof(FrameBuffer));
	
	for (int y = 0; y < WINDOWY; y++) {
		for (int x = 0; x < WINDOWX; x++) {
			
			switch (GBCore::gpu._Screen[x][y]) {
			case 0:
				color = 255;
				break;
			case 1:
				color = 187;
				break;
			case 2:
				color = 97;
				break;
			case 3:
				color = 0;
				break;
			}
			FrameBuffer[y * WINDOWX * 3 + x * 3] = color;//b,g,r
			FrameBuffer[y * WINDOWX * 3 + x * 3+1] = color;
			FrameBuffer[y * WINDOWX * 3 + x * 3+2] = color;
		}
	}
}
void OpenFile(HWND hWnd) {
	OPENFILENAME ofn;
	TCHAR szFileName[MAX_PATH] = _T("");
	char FileName[MAX_PATH]= ("");
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;   
	ofn.lpstrFilter = _T("gbROM (*.gb)\0*.gb\0All Files (*.*)\0*.*\0"); 
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("gb");        
	ofn.lpstrInitialDir = _T(".\\");     //current dir.

	if (GetOpenFileName(&ofn))
	{
		int iLength= WideCharToMultiByte(CP_ACP, 0, szFileName, -1, NULL, 0, NULL, NULL);
		//szFileName 
		WideCharToMultiByte(CP_ACP, 0, szFileName, -1, FileName, iLength, NULL, NULL);
		GBCore::memory.LoadRom(FileName);
		RomLoaded = 1;
		GameBoyInit();
		// do something   
	}
}
