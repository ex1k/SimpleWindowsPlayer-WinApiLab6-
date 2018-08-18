
#include "stdafx.h"
#include "WinApiLab6.h"
#include <Vfw.h>
#pragma comment(lib, "vfw32.lib")

HINSTANCE hInst; 

int music, volume_number, volume, song_count;
BITMAP bm;
HBITMAP hMask, hBmpAllMask, hBmpFon;
BOOL hand, play;
HWND hWndMCI;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


class MyControl
{
private:
	HBITMAP hBmpFonCtrl, hBmpMaskCtrl, hMaskCtrl1;
	BITMAP bmCtrl;
	unsigned char* hAllBitsCtrl;
	int width;
public:
	MyControl(LPCWSTR img, LPCWSTR mask)
	{
		int pixel, mByte;
		unsigned char add;
		hMaskCtrl1 = (HBITMAP)LoadImage(hInst, mask, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hMaskCtrl1, sizeof(bmCtrl), &bmCtrl);
		BYTE* pMaskBits = (BYTE*)bmCtrl.bmBits;
		width = (bmCtrl.bmWidth % 16 == 0 ? bmCtrl.bmWidth / 16 : bmCtrl.bmWidth / 16 + 1) * 2;
		hAllBitsCtrl = new unsigned char[bmCtrl.bmHeight * width];
		memset(hAllBitsCtrl, 0, sizeof(unsigned char) * bmCtrl.bmHeight * width);
		for (int i = 0; i < bmCtrl.bmHeight; i++)
		{
			for (int j = 0; j < bmCtrl.bmWidth; j++)
			{
				pixel = pMaskBits[i*bmCtrl.bmWidthBytes + j];
				if (pixel == 0)
				{
					mByte = (bmCtrl.bmHeight - i - 1)*width + j / 8;
					add = 1 << (7 - (j % 8));
					hAllBitsCtrl[mByte] += add;
				}
			}
		}
		hBmpMaskCtrl = CreateBitmap(bmCtrl.bmWidth, bmCtrl.bmHeight, 1, 1, hAllBitsCtrl);
		if (img == NULL)
			hBmpFonCtrl = NULL;
		else
			hBmpFonCtrl = (HBITMAP)LoadImage(NULL, img, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	}
public:
	BOOL CheckPoint(int x, int y)
	{
		if ((hAllBitsCtrl[y*width + x / 8] >> (7 - (x % 8))) % 2 == 0)
			return TRUE;
		return FALSE;
	}
	void ControlPress(HDC hdcComp, HDC hdcBuffer)
	{
		if (hBmpFonCtrl != NULL)
		{
			SelectObject(hdcComp, hBmpFonCtrl);
			BitBlt(hdcBuffer, 0, 0, bmCtrl.bmWidth, bmCtrl.bmHeight, hdcComp, 0, 0, SRCINVERT);
			SelectObject(hdcComp, hBmpMaskCtrl);
			BitBlt(hdcBuffer, 0, 0, bmCtrl.bmWidth, bmCtrl.bmHeight, hdcComp, 0, 0, SRCAND);
			SelectObject(hdcComp, hBmpFonCtrl);
			BitBlt(hdcBuffer, 0, 0, bmCtrl.bmWidth, bmCtrl.bmHeight, hdcComp, 0, 0, SRCINVERT);
		}
	}
};
void get_video_size(HWND hWnd) {
	RECT rc;
	GetClientRect(hWnd, &rc);
	MoveWindow(hWndMCI, rc.top+135, rc.left+100, rc.right-280, rc.bottom - 245, TRUE);
}
MyControl* control[10];

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
 
	volume = 1000;
   
    MyRegisterClass(hInstance);


    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPILAB6));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPILAB6);
    wcex.lpszClassName  = L"Lab6";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd;
	HRGN hRgn = 0;
	int pixel, xStart, width, mByte;
	unsigned char* pAllBits, add;
	hWnd = CreateWindow(L"Lab6", L"Lab6", WS_POPUP, 100, 100, 740, 520, NULL, NULL, hInstance, NULL);
	if (!hWnd) {
		return FALSE;
	}
	hMask = (HBITMAP)LoadImage(hInst, L"source_mask.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	GetObject(hMask, sizeof(bm), &bm);
	BYTE* pMaskBits = (BYTE*)bm.bmBits;
	width = (bm.bmWidth % 16 == 0 ? bm.bmWidth / 16 : bm.bmWidth / 16 + 1) * 2;
	pAllBits = new unsigned char[bm.bmHeight * width];
	memset(pAllBits, 0, sizeof(unsigned char) * bm.bmHeight * width);
	xStart = -1;
	for (int i = 0; i < bm.bmHeight; i++)
	{
		for (int j = 0; j < bm.bmWidth; j++)
		{
			pixel = pMaskBits[i*bm.bmWidthBytes + j];
			if (pixel != 0)
			{
				mByte = (bm.bmHeight - i - 1)*width + j / 8;
				add = 1 << (7 - (j % 8));
				pAllBits[mByte] += add;
				if (xStart == -1) xStart = j;
			}
			else
			{
				if (xStart != -1)
				{
					if (hRgn == 0)
						hRgn = CreateRectRgn(xStart, bm.bmHeight - i - 1, j, bm.bmHeight - i);
					else CombineRgn(hRgn, hRgn, CreateRectRgn(xStart, bm.bmHeight - i - 1, j, bm.bmHeight - i), RGN_OR);
					xStart = -1;
				}
			}
			if (j == bm.bmWidth - 1 && xStart != -1)
			{
				if (hRgn == 0)
					hRgn = CreateRectRgn(xStart, bm.bmHeight - i - 1, j + 1, bm.bmHeight - i);
				else CombineRgn(hRgn, hRgn, CreateRectRgn(xStart, bm.bmHeight - i - 1, j + 1, bm.bmHeight - i), RGN_OR);
				xStart = -1;
			}
		}
	}
	SetWindowRgn(hWnd, hRgn, TRUE);
	hBmpAllMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, pAllBits);
	hBmpFon = (HBITMAP)LoadImage(NULL, L"source.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	control[0] = new MyControl(L"source_play.bmp", L"source_play_mask.bmp");
	control[1] = new MyControl(L"source_stop.bmp", L"source_stop_mask.bmp");
	control[2] = new MyControl(NULL, L"source_cancel_mask.bmp");
	control[3] = new MyControl(L"source_vol1.bmp", L"source_vol1_mask.bmp");
	control[4] = new MyControl(L"source_vol2.bmp", L"source_vol2_mask.bmp");
	control[5] = new MyControl(L"source_vol3.bmp", L"source_vol3_mask.bmp");
	control[6] = new MyControl(L"source_vol4.bmp", L"source_vol4_mask.bmp");
	control[7] = new MyControl(L"source_vol5.bmp", L"source_vol5_mask.bmp");
	control[8] = new MyControl(NULL, L"source_open_mask.bmp");
	control[9] = new MyControl(NULL, L"source_close_mask.bmp");
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc, hdcBuffer, compHdc;
	HBITMAP hBmpBuffer;
	int x, y, wmId, wmEvent, j;
	switch (message)
    {
	case WM_CREATE:
		play = FALSE;
		volume_number = 1;
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		if (control[0]->CheckPoint(x, y))
		{
			play = TRUE;
			if (MCIWndCanPlay(hWndMCI)) {
				MCIWndPlay(hWndMCI);
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[1]->CheckPoint(x, y))
		{
			play = FALSE;
			MCIWndPause(hWndMCI);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[2]->CheckPoint(x, y))
		{
			MCIWndDestroy(hWndMCI);
		}
		if (control[3]->CheckPoint(x, y)) {
			volume_number = 1;
			volume = 500;
			MCIWndSetVolume(hWndMCI, volume);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[4]->CheckPoint(x, y)) {
			volume_number = 2;
			volume = 600;
			MCIWndSetVolume(hWndMCI, volume);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[5]->CheckPoint(x, y)) {
			volume_number = 3;
			volume = 700;
			MCIWndSetVolume(hWndMCI, volume);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[6]->CheckPoint(x, y)) {
			volume_number = 4;
			volume = 800;
			MCIWndSetVolume(hWndMCI, volume);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[7]->CheckPoint(x, y)) {
			volume_number = 5;
			volume = 900;
			MCIWndSetVolume(hWndMCI, volume);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[8]->CheckPoint(x, y)) {
			song_count++;
			if (song_count > 1) {
				MCIWndClose(hWndMCI);
				song_count--;
			}
			hWndMCI = MCIWndCreate(hWnd, hInst, MCIWNDF_NOPLAYBAR, _T(""));
			MCIWndOpenDialog(hWndMCI);
			get_video_size(hWnd);
			MCIWndPlay(hWndMCI);
			MCIWndSetVolume(hWndMCI, volume);
			play = TRUE;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (control[9]->CheckPoint(x, y)) {
			DestroyWindow(hWnd);
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
			ReleaseCapture();
			SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		else
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			for (int i = 0; i < 10; i++)
				if (control[i]->CheckPoint(x, y)) {
					hand = TRUE;
					break;
				}
			if (hand)
				SetCursor(LoadCursor(NULL, IDC_HAND));
			else 
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			hand = FALSE;
		}
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {        
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			hdc = BeginPaint(hWnd, &ps);
			hdcBuffer = CreateCompatibleDC(hdc);
			hBmpBuffer = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
			SelectObject(hdcBuffer, hBmpBuffer);
			compHdc = CreateCompatibleDC(hdc);
			SelectObject(compHdc, hBmpFon);
			MaskBlt(hdcBuffer, 0, 0, bm.bmWidth, bm.bmHeight, compHdc, 0, 0, hBmpAllMask, 0, 0, SRCCOPY);
			if (play == TRUE) {
				control[0]->ControlPress(compHdc, hdcBuffer);
				control[1]->ControlPress(compHdc, hdcBuffer);
			}
			for (int i = 3; i < 3 + volume_number; i++) {
				control[i]->ControlPress(compHdc, hdcBuffer);
			}
			BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcBuffer, 0, 0, SRCCOPY);
			DeleteDC(hdcBuffer);
			DeleteObject(hBmpBuffer);
			DeleteDC(compHdc);
			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

