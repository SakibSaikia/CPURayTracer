#include "stdafx.h"

constexpr float AspectRatio()
{
	return static_cast<float>(AppSettings::k_backbufferWidth) / static_cast<float>(AppSettings::k_backbufferHeight);
}

Camera::Camera() :
	m_origin{ XMVectorSet(0.f, 0.f, 0.f, 1.f) },
	m_x{ XMVectorSet(2.f * AspectRatio(), 0.f, 0.f, 0.f) },
	m_y{ XMVectorSet(0.f, 2.f, 0.f, 0.f) },
	m_lowerLeft{ XMVectorSet(-AspectRatio(), -1.f, -1.f, 1.f) }
{
}

Ray Camera::GetRay(float u, float v) const
{
	XMVECTOR p = m_lowerLeft + u * m_x + v * m_y;
	return Ray{ m_origin, p - m_origin };
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

	{
		auto start = std::chrono::high_resolution_clock::now();

		size_t rayCount = DrawBitmap(hWnd);

		auto stop = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = stop - start;
		double timeElapsed = duration.count();

		DisplayStats(hWnd, rayCount, timeElapsed);
	}

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
	std::fill(m_backbufferHdr.begin(), m_backbufferHdr.end(), DirectX::XMVectorZero());

	m_backbufferLdr.resize(AppSettings::k_backbufferWidth * AppSettings::k_backbufferHeight);
	std::fill(m_backbufferLdr.begin(), m_backbufferLdr.end(), 0);
}

std::pair<float, float> RayTracingApp::GetJitterOffset() const
{
	static std::random_device device;
	static std::mt19937 generator(device());
	static std::uniform_real_distribution<float> uniformDist(0.f, 1.f);

	return std::make_pair(uniformDist(generator), uniformDist(generator));
}

std::vector<Ray> RayTracingApp::GenerateRays() const
{
	std::vector<Ray> rays;
	rays.reserve(AppSettings::k_backbufferWidth * AppSettings::k_backbufferHeight);

	auto xsize = static_cast<float>(AppSettings::k_backbufferWidth);
	auto ysize = static_cast<float>(AppSettings::k_backbufferHeight);

	auto[xOffset, yOffset] = GetJitterOffset();

	for (auto j = AppSettings::k_backbufferHeight - 1; j >= 0; --j)
	{
		for (auto i = 0; i < AppSettings::k_backbufferWidth; ++i)
		{
			float u = static_cast<float>(i + xOffset) / xsize;
			float v = static_cast<float>(j + yOffset) / ysize;

			rays.push_back(m_camera.GetRay(u, v));
		}
	}

	return rays;
}

size_t RayTracingApp::DrawBitmap(HWND hWnd)
{
	using namespace DirectX;
	using namespace DirectX::PackedVector;

	++m_sampleCount;

	// Rays
	std::vector<Ray> rayBuffer = GenerateRays();

	// Trace
	for (auto rayIndex = 0; rayIndex < rayBuffer.size(); ++rayIndex)
	{
		const Ray& r = rayBuffer[rayIndex];

		XMVECTOR& colorVec = m_backbufferHdr[rayIndex];
		colorVec += GetSceneColor(r);
	}

	// Tonemap & gamma correction
	std::transform(
		m_backbufferHdr.cbegin(), m_backbufferHdr.cend(),
		m_backbufferLdr.begin(),
		[n = m_sampleCount](const DirectX::XMVECTOR& hdrColor) -> XMCOLOR
	{
		XMVECTOR avgColor = hdrColor / static_cast<float>(n);

		avgColor = XMVectorSqrtEst(avgColor);

		XMCOLOR outColor;
		XMStoreColor(&outColor, avgColor);

		return outColor;
	});


	m_backbufferBitmap->CopyFromMemory(nullptr, m_backbufferLdr.data(), sizeof(m_backbufferLdr[0]) * AppSettings::k_backbufferWidth);

	m_renderTarget->DrawBitmap(m_backbufferBitmap.Get());

	return rayBuffer.size();
}

std::optional<Payload> RayTracingApp::GetClosestIntersection(const Ray& ray) const
{
	Payload payload;
	bool hitAnything = false;
	XMVECTOR tClosest = XMVectorReplicate(FLT_MAX);
	static const XMVECTORF32 bias{ 0.001, 0.001, 0.001 };

	for (const Sphere& s : m_scene)
	{
		if (s.Intersect(ray, bias, tClosest, payload))
		{
			tClosest = payload.t;
			hitAnything = true;
		}
	}

	if (hitAnything)
	{
		return payload;
	}
	else
	{
		return std::nullopt;
	}
}

XMVECTOR RayTracingApp::GetSceneColor(const Ray& ray) const
{
	static const XMVECTORF32 half{ 0.5f, 0.5f, 0.5f };

	if (auto hitInfo = GetClosestIntersection(ray))
	{
		const Payload& hit = hitInfo.value();

		XMVECTOR target = hit.p + hit.normal + GetRandomVectorInUnitSphere();

		Ray newRay{ hit.p, target - hit.p };

		return 0.5 * GetSceneColor(newRay);
	}
	else
	{
		// Sky gradient
		XMVECTOR rayDir = XMVector3Normalize(ray.direction);
		float t = 0.5f * (XMVectorGetY(rayDir) + 1.f);

		return (1.f - t) * XMVECTORF32 { 1.f, 1.f, 1.f } +t * XMVECTORF32{ 0.5f, 0.7f, 1.f };
	}
}

void RayTracingApp::DisplayStats(HWND hWnd, const size_t rayCount, const double timeElapsed) const
{
	double mraysPerSecond = static_cast<double>(rayCount) / timeElapsed;

	std::wstring windowText = AppSettings::k_windowCaption +
		L"\t Mrays/s: " + std::to_wstring(mraysPerSecond) +
		L"\t spp: " + std::to_wstring(m_sampleCount);

	SetWindowText(hWnd, windowText.c_str());
}