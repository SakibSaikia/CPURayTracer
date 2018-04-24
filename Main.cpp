// GDI code adapted from https://github.com/aras-p/ToyPathTracer

#include "stdafx.h"

constexpr int k_backbufferWidth = 1280;
constexpr int k_backbufferHeight = 720;
constexpr wchar_t k_windowName[] = L"DemoWindow";

static HWND g_wndHandle = nullptr;
static std::vector<float> g_backbufferHdr;
static std::vector<uint32_t> g_backbufferLdr;
static HBITMAP g_backbufferBitmap;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitDemo(HINSTANCE instanceHandle, int show);
int Run();
void InitBackbufferBitmap();
void DrawBitmap(HDC dc, int width, int height);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	InitDemo(hInstance, nShowCmd);
	return Run();
}

void InitDemo(HINSTANCE instanceHandle, int show)
{
	WNDCLASS desc;
	desc.style = CS_HREDRAW | CS_VREDRAW;				
	desc.lpfnWndProc = WndProc;							
	desc.cbClsExtra = 0;								
	desc.cbWndExtra = 0;								
	desc.hInstance = instanceHandle;					
	desc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);	
	desc.hCursor = LoadCursor(nullptr, IDC_ARROW);		
	desc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW); 
	desc.lpszMenuName = nullptr;						
	desc.lpszClassName = k_windowName;
	RegisterClass(&desc);

	g_wndHandle = CreateWindow(
		k_windowName,
		L"Raytracing Demo",									
		WS_OVERLAPPEDWINDOW,							
		CW_USEDEFAULT,									
		CW_USEDEFAULT,									
		k_backbufferWidth,								
		k_backbufferHeight,								
		nullptr,										
		nullptr,										
		instanceHandle,									
		nullptr											
	);

	assert(g_wndHandle != nullptr && L"Failed to create window");

	InitBackbufferBitmap();

	ShowWindow(g_wndHandle, show);
	UpdateWindow(g_wndHandle);
}

int Run()
{
	MSG msg = { nullptr };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			InvalidateRect(g_wndHandle, nullptr, FALSE);
			UpdateWindow(g_wndHandle);
		}
	}

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rect;
		HDC hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);
		DrawBitmap(hdc, rect.right, rect.bottom);
	}
	return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(g_wndHandle);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void InitBackbufferBitmap()
{
	g_backbufferHdr.resize(k_backbufferWidth * k_backbufferHeight * 4);
	std::fill(g_backbufferHdr.begin(), g_backbufferHdr.end(), 0.f);

	g_backbufferLdr.resize(k_backbufferWidth * k_backbufferHeight);
	std::fill(g_backbufferLdr.begin(), g_backbufferLdr.end(), 0);

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = k_backbufferWidth;
	bmi.bmiHeader.biHeight = k_backbufferHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = k_backbufferWidth * k_backbufferHeight * 4;
	HDC hdc = CreateCompatibleDC(GetDC(nullptr));

	g_backbufferBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&g_backbufferLdr.at(0)), nullptr, 0x0);
}

void DrawBitmap(HDC dc, int width, int height)
{
	std::generate(g_backbufferLdr.begin(), g_backbufferLdr.end(), 
		[src = g_backbufferHdr.data()]() mutable
		{
			auto r = std::min<uint32_t>(static_cast<uint32_t>(src[0] * 255.9f), 255u);
			auto g = std::min<uint32_t>(static_cast<uint32_t>(src[1] * 255.9f), 255u);
			auto b = std::min<uint32_t>(static_cast<uint32_t>(src[2] * 255.9f), 255u);

			src += 4;

			return b | (g << 8) | (r << 16);
		});


	HDC srcDC = CreateCompatibleDC(dc);
	SetStretchBltMode(dc, COLORONCOLOR);
	SelectObject(srcDC, g_backbufferBitmap);
	StretchBlt(dc, 0, 0, width, height, srcDC, 0, 0, k_backbufferWidth, k_backbufferHeight, SRCCOPY);
	DeleteObject(srcDC);
}
