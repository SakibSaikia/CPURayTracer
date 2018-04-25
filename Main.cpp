#include "stdafx.h"
#pragma comment(lib, "d2d1")

constexpr int k_backbufferWidth = 1280;
constexpr int k_backbufferHeight = 720;
constexpr wchar_t k_windowName[] = L"DemoWindow";

namespace
{
	HWND g_wndHandle = nullptr;
	bool g_initialized = false;
	std::vector<float> g_backbufferHdr;
	std::vector<uint32_t> g_backbufferLdr;
	Microsoft::WRL::ComPtr<ID2D1Factory> g_factory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> g_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> g_renderTarget;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitDemo(HINSTANCE instanceHandle, int show);
int Run();
void InitBackbufferBitmap();
void DrawBitmap();


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
	case WM_CREATE:
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, g_factory.GetAddressOf());
		assert(hr == S_OK);
	}
	case WM_PAINT:
		if (g_initialized)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			g_renderTarget->BeginDraw();
			g_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
			DrawBitmap();
			g_renderTarget->EndDraw();

			EndPaint(hWnd, &ps);
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
	// Factory
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, g_factory.GetAddressOf());
	assert(hr == S_OK);

	// Render target
	RECT rc;
	GetClientRect(g_wndHandle, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	hr = g_factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(g_wndHandle, size),
		g_renderTarget.GetAddressOf()
	);

	assert(hr == S_OK);

	// Bitmap
	FLOAT dpiX, dpiY;
	g_factory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_BITMAP_PROPERTIES desc = {};
	desc.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	desc.dpiX = dpiX;
	desc.dpiY = dpiY;

	hr = g_renderTarget->CreateBitmap(size, desc, g_backbufferBitmap.GetAddressOf());
	assert(hr == S_OK);

	g_backbufferHdr.resize(k_backbufferWidth * k_backbufferHeight * 4);
	std::fill(g_backbufferHdr.begin(), g_backbufferHdr.end(), 0.f);

	g_backbufferLdr.resize(k_backbufferWidth * k_backbufferHeight);
	std::fill(g_backbufferLdr.begin(), g_backbufferLdr.end(), 0);

	g_initialized = true;
}

void DrawBitmap()
{
	std::generate(g_backbufferLdr.begin(), g_backbufferLdr.end(), 
		[src = g_backbufferHdr.begin()]() mutable
		{
			auto r = std::min<uint32_t>(static_cast<uint32_t>(*src++ * 255.9f), 255u);
			auto g = std::min<uint32_t>(static_cast<uint32_t>(*src++ * 255.9f), 255u);
			auto b = std::min<uint32_t>(static_cast<uint32_t>(*src++ * 255.9f), 255u);

			src++;

			return b | (g << 8) | (r << 16);
		});

	g_backbufferBitmap->CopyFromMemory(nullptr, g_backbufferLdr.data(), sizeof(uint32_t) * k_backbufferWidth);

	g_renderTarget->DrawBitmap(g_backbufferBitmap.Get());
}
