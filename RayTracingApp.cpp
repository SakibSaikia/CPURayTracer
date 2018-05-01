#include "stdafx.h"

constexpr float AspectRatio()
{
	return static_cast<float>(AppSettings::k_backbufferWidth) / static_cast<float>(AppSettings::k_backbufferHeight);
}

void RayTracingApp::OnInitialize(HWND hWnd)
{
	InitDirect2D(hWnd);
	InitBuffers();
}

void RayTracingApp::OnRender(HWND hWnd)
{
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hWnd, &ps);
	m_renderTarget->BeginDraw();

	m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

	DrawBitmap();

	m_renderTarget->EndDraw();
	EndPaint(hWnd, &ps);
}

void RayTracingApp::InitDirect2D(HWND hWnd)
{
	// Factory
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.GetAddressOf());
	assert(hr == S_OK);

	// Render target
	RECT rc;
	GetClientRect(hWnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

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
	m_backbufferHdr.resize(AppSettings::k_backbufferWidth * AppSettings::k_backbufferHeight);
	std::fill(m_backbufferHdr.begin(), m_backbufferHdr.end(), DirectX::XMFLOAT3(0.f, 0.f, 0.f));

	m_backbufferLdr.resize(AppSettings::k_backbufferWidth * AppSettings::k_backbufferHeight);
	std::fill(m_backbufferLdr.begin(), m_backbufferLdr.end(), 0);
}

void RayTracingApp::DrawBitmap()
{
	using namespace DirectX;
	using namespace DirectX::PackedVector;

	// Rays
	XMVECTORF32 ll{ -AspectRatio(), -1.f, 1.f };
	XMVECTORF32 x{ 2.f * AspectRatio(), 0.f, 0.f };
	XMVECTORF32 y{ 0.f, 2.f, 0.f };
	XMVECTORF32 o{ 0.f, 0.f, 0.f };

	auto xsize = static_cast<float>(AppSettings::k_backbufferWidth);
	auto ysize = static_cast<float>(AppSettings::k_backbufferHeight);

	std::vector<Ray> rays;
	for (auto j = AppSettings::k_backbufferHeight - 1; j >= 0; --j)
	{
		for (auto i = 0; i < AppSettings::k_backbufferWidth; ++i)
		{
			float u = static_cast<float>(i) / xsize;
			float v = static_cast<float>(j) / ysize;

			rays.emplace_back(o, ll + u * x + v * y);
		}
	}

	// Trace
	Sphere s{ XMVECTORF32{0.f, 0.f, 1.f}, 0.5 };
	std::transform(rays.cbegin(), rays.cend(), m_backbufferHdr.begin(),
		[&s](const Ray& r) -> XMFLOAT3
		{
			if (s.Intersect(r))
			{
				return XMFLOAT3{ 1.f,0.f,0.f };
			}
			else
			{
				XMVECTOR rayDir = XMVector3Normalize(r.direction);
				float t = 0.5f * (XMVectorGetY(rayDir) + 1.f);

				XMFLOAT3 outColor;
				XMStoreFloat3(&outColor, (1.f - t) * XMVECTORF32 { 1.f, 1.f, 1.f } +t * XMVECTORF32{ 0.5f, 0.7f, 1.f });

				return outColor;
			}
		});

	// Tonemap
	std::transform(m_backbufferHdr.cbegin(), m_backbufferHdr.cend(), m_backbufferLdr.begin(),
		[](const DirectX::XMFLOAT3& hdrColor) -> XMCOLOR
		{
			auto r = std::min<uint32_t>(static_cast<uint32_t>(hdrColor.x * 255.9f), 255u);
			auto g = std::min<uint32_t>(static_cast<uint32_t>(hdrColor.y * 255.9f), 255u);
			auto b = std::min<uint32_t>(static_cast<uint32_t>(hdrColor.z * 255.9f), 255u);

			return b | (g << 8) | (r << 16);
		});

	m_backbufferBitmap->CopyFromMemory(nullptr, m_backbufferLdr.data(), sizeof(m_backbufferLdr[0]) * AppSettings::k_backbufferWidth);

	m_renderTarget->DrawBitmap(m_backbufferBitmap.Get());
}