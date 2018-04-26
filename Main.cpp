#include "stdafx.h"

#pragma comment(lib, "d2d1")

namespace
{
	constexpr wchar_t k_windowName[] = L"AppWindow";

	HWND g_wndHandle = nullptr;
	bool g_initialized = false;
	RayTracingApp g_app;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitWindow(HINSTANCE instanceHandle, int show);
int Run();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	InitWindow(hInstance, nShowCmd);
	return Run();
}

void InitWindow(HINSTANCE instanceHandle, int show)
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
		L"Demo",									
		WS_OVERLAPPEDWINDOW,							
		CW_USEDEFAULT,									
		CW_USEDEFAULT,									
		AppSettings::k_backbufferWidth,								
		AppSettings::k_backbufferHeight,								
		nullptr,										
		nullptr,										
		instanceHandle,									
		nullptr											
	);

	assert(g_wndHandle != nullptr && L"Failed to create window");

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
	case WM_CREATE:
		g_app.OnInitialize(hWnd);
		return 0;
	case WM_PAINT:
		g_app.OnRender(hWnd);
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
