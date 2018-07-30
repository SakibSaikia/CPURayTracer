#include "app.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void RayTracingApp::Initialize(HINSTANCE instanceHandle, int show)
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
	desc.lpszClassName = GetWindowName().c_str();
	RegisterClass(&desc);

	m_wndHandle = CreateWindow(
		GetWindowName().c_str(),
		GetWindowName().c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		GetBackBufferWidth(),
		GetBackBufferHeight(),
		nullptr,
		nullptr,
		instanceHandle,
		nullptr
	);

	assert(m_wndHandle != nullptr && L"Failed to create window");

	ShowWindow(m_wndHandle, show);
	UpdateWindow(m_wndHandle);

	InitDirect2D(m_wndHandle);
	InitBuffers();
	RandGenerator::Init();

	OnInitialize(m_wndHandle);
}

int RayTracingApp::Run() noexcept
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
			OnRender(m_wndHandle);
			InvalidateRect(m_wndHandle, nullptr, FALSE);
			UpdateWindow(m_wndHandle);
		}
	}

	return static_cast<int>(msg.wParam);
}

void RayTracingApp::InitDirect2D(HWND hWnd) noexcept
{
	// Factory
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.GetAddressOf());
	assert(hr == S_OK);

	// Render target
	RECT rc;
	GetClientRect(hWnd, &rc);

	const D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

	hr = m_d2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, size),
		m_renderTarget.GetAddressOf()
	);

	assert(hr == S_OK);

	// Bitmap
	FLOAT dpiX, dpiY;
	m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_BITMAP_PROPERTIES desc = {};
	desc.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	desc.dpiX = dpiX;
	desc.dpiY = dpiY;

	hr = m_renderTarget->CreateBitmap(size, desc, m_backbufferBitmap.GetAddressOf());
	assert(hr == S_OK);
}

void RayTracingApp::InitBuffers()
{
	m_backbufferHdr.resize(GetBackBufferWidth() * GetBackBufferHeight());
	std::fill(m_backbufferHdr.begin(), m_backbufferHdr.end(), DirectX::XMVectorZero());

	m_backbufferLdr.resize(GetBackBufferWidth() * GetBackBufferHeight());
	std::fill(m_backbufferLdr.begin(), m_backbufferLdr.end(), 0);
}