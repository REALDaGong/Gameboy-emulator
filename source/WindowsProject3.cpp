//WindowsProject3.cpp: 定义应用程序的入口点。

#include "stdafx.h"
#include "WindowsProject3.h"
#pragma comment(lib,"Winmm.lib")
#include "commdlg.h"
#include <MMSystem.h>
#include "GB_Main.h"
#define MAX_LOADSTRING 100
const int WINDOWX = 160;
const int WINDOWY = 144;
const int bit = 24;
const int TIMER_ID = 1;
int Frame = 0;
// 全局变量: 
HINSTANCE hInst;                                // 当前实例 进程ID
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本 
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
int TimerID;                                    // 计时器控制速度
static HDC screen_hdc;							// 绘图区域
static HWND screen_hwnd;						// 窗口实例 句柄ID
static HDC hCompatibleDC;				// 兼容HDC 内存绘图区域并复制到显示区  
static HBITMAP hCompatibleBitmap;               //数据无关位图 兼容BITMAP  
static HBITMAP hOldBitmap;						//旧的BITMAP 上一张图                   
static BITMAPINFO binfo;						//BITMAPINFO结构体  
static BYTE FrameBuffer[WINDOWX*WINDOWY*3];		//已处理的图像数据缓存
static BOOL RomLoaded = 0;						//是否载入成功

// 此代码模块中包含的函数的前向声明:	
void         OpenFile(HWND hwnd);  //打开游戏

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

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	
    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;


    }
	
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT3));

    MSG msg;

	//TimerID = timeSetEvent(16,1,RunANewFrame(),NULL,TIME_PERIODIC);
	
	
	// 主消息循环: 
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


void OpenFile(HWND hWnd) {
	OPENFILENAME ofn;								
	TCHAR szFileName[MAX_PATH] = _T("");				//文件名
	char FileName[MAX_PATH] = ("");						//文件名
	ZeroMemory(&ofn, sizeof(ofn));						//清空结构体
	ofn.lStructSize = sizeof(ofn);						//指定结构体大小
	ofn.hwndOwner = hWnd;    //父窗口句柄  
	ofn.lpstrFilter = _T("gbROM (*.gb)\0*.gb\0All Files (*.*)\0*.*\0");   
	//打开的文件类型，这里以gb和所有文件为例  
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("gb");        //默认的打开的文件类型  
	ofn.lpstrInitialDir = _T(".\\");     //默认的打开的文件路径，这里以当前目录为例  

	if (GetOpenFileName(&ofn))
	{
		int iLength = WideCharToMultiByte(CP_ACP, 0, szFileName, -1, NULL, 0, NULL, NULL);
		//szFileName为获取的文件名  
		WideCharToMultiByte(CP_ACP, 0, szFileName, -1, FileName, iLength, NULL, NULL);
		memory.LoadRom(FileName);
		RomLoaded = 1;
		GameBoyInit();
		// do something   
	}
}
//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
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

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

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

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE: 
	{
		UINT_PTR ret = SetTimer(hWnd, TIMER_ID,16,NULL);

		ZeroMemory(&binfo, sizeof(BITMAPINFO));
		binfo.bmiHeader.biBitCount = 24;      //每个像素多少位，也可直接写24(RGB)或者32(RGBA)  
		binfo.bmiHeader.biCompression = BI_RGB;
		binfo.bmiHeader.biHeight = -144;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSizeImage = 0;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biWidth = 160;

		ZeroMemory(FrameBuffer, sizeof(FrameBuffer));
		//获取屏幕HDC  
		screen_hwnd = hWnd;
		screen_hdc = GetDC(screen_hwnd);

		//获取兼容HDC和兼容Bitmap,兼容Bitmap选入兼容HDC(每个HDC内存每时刻仅能选入一个GDI资源,GDI资源要选入HDC才能进行绘制)  
		hCompatibleDC = CreateCompatibleDC(screen_hdc);
		hCompatibleBitmap = CreateCompatibleBitmap(screen_hdc, WINDOWX, WINDOWY);
		hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hCompatibleBitmap);
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
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
            // TODO: 在此处添加使用 hdc 的任何绘图代码...

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
	{

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
	{

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

// “关于”框的消息处理程序。
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
	//将颜色数据打印到屏幕上
	//SetDIBits(screen_hdc, hCompatibleBitmap, 0, WINDOWX, FrameBuffer, (BITMAPINFO*)&binfo, DIB_RGB_COLORS);
	//BitBlt(screen_hdc, 0, 0, WINDOWX+30, WINDOWY+30, hCompatibleDC, 0, 0, SRCCOPY);
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int i=StretchDIBits(screen_hdc,0,0,rect.right-rect.left,rect.bottom-rect.top,0,0,WINDOWX,WINDOWY,FrameBuffer,&binfo, DIB_RGB_COLORS, SRCCOPY);
	return 1;
}

void Translate() {
	int color = 0;
	memset(FrameBuffer, 0, sizeof(FrameBuffer));
	//int Xoffset = memory.MemoryRead(SCX);
	//int Yoffset = memory.MemoryRead(SCY);
	for (int y = 0; y < WINDOWY; y++) {
		for (int x = 0; x < WINDOWX; x++) {
			
			switch (gpu._Screen[x][y]) {
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
			FrameBuffer[y * WINDOWX * 3 + x * 3] = color;//b,g,r wtf?
			FrameBuffer[y * WINDOWX * 3 + x * 3+1] = color;
			FrameBuffer[y * WINDOWX * 3 + x * 3+2] = color;
		}
	}
}
