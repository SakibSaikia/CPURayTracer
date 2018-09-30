#include "spheres-app.h"
#include <sstream>

void SpheresApp::OnInitialize(HWND hWnd)
{
	InitCamera();
	InitScene();
}

void SpheresApp::OnRender(HWND hWnd)
{
	PAINTSTRUCT ps;

	const HDC hdc = BeginPaint(hWnd, &ps);
	m_renderTarget->BeginDraw();

	m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

	{
		const auto start = std::chrono::high_resolution_clock::now();

		const size_t rayCount = DrawBitmap(hWnd);

		const auto stop = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::micro> duration = stop - start;
		const double timeElapsed = duration.count();

		DisplayStats(hWnd, rayCount, timeElapsed);
	}

	m_renderTarget->EndDraw();
	EndPaint(hWnd, &ps);
}

void SpheresApp::InitCamera()
{
	XMVECTOR camOrigin = XMVectorSet(12.f, 2.f, -2.5f, 1.f);
	XMVECTOR camLookAt = XMVectorSet(0, 1, 0, 1.f);

	m_camera = std::make_unique<Camera>(
		camOrigin,
		camLookAt,
		AppSettings::k_verticalFov,
		AppSettings::k_aspectRatio,
		XMVectorGetX(XMVector3Length(camOrigin - camLookAt)),
		AppSettings::k_aperture);

	m_exposure = -15;
}

void SpheresApp::InitScene()
{
	static std::random_device device;
	static std::ranlux24_base generator(device());
	static std::uniform_real_distribution<float> uniformDist(0.f, 1.f);

	m_scene.reserve(500);

	// Floor
	m_textures.push_back(std::make_unique<CheckerTexture>(XMCOLOR{ 0.9f, 0.9f, 0.9f, 1.f }, XMCOLOR{ 0.2f, 0.3f, 0.1f, 1.f }, 2500.f));
	m_scene.push_back(std::make_unique<Sphere>(XMVECTORF32{ 0, -1000, 0 }, 1000.f, std::make_unique<DielectricOpaque>(m_textures.back().get(), XMVectorReplicate(16.f))));

	// Random small spheres
	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			const float chooseMat = uniformDist(generator);
			XMVECTORF32 center{ a + 0.9f * uniformDist(generator), 0.2f, b + 0.9f * uniformDist(generator) };

			if (chooseMat < 0.8f)
			{
				m_textures.push_back(std::make_unique<ConstTexture>(
					XMCOLOR{
						uniformDist(generator) * uniformDist(generator),
						uniformDist(generator) * uniformDist(generator),
						uniformDist(generator) * uniformDist(generator),
						1.f
					}));

				float smoothness = 8.f * (4.f + uniformDist(generator));

				m_scene.push_back(std::make_unique<Sphere>(center, 0.2f, std::make_unique<DielectricOpaque>(m_textures.back().get(), XMVectorReplicate(smoothness))));
			}
			else if (chooseMat < 0.95f)
			{
				m_textures.push_back(std::make_unique<ConstTexture>(
					XMCOLOR{
						0.5f * (1.f + uniformDist(generator)),
						0.5f * (1.f + uniformDist(generator)),
						0.5f * (1.f + uniformDist(generator)),
						1.f
					}
				));

				m_scene.push_back(std::make_unique<Sphere>(center, 0.2f, std::make_unique<Metal>(m_textures.back().get(), XM_Zero)));
			}
			else
			{
				float smoothness = 8.f * (4.f + uniformDist(generator));

				m_scene.emplace_back(std::make_unique<Sphere>(center, 0.2f, std::make_unique<DielectricTransparent>(XMVectorReplicate(smoothness), 1.5f)));
			}
		}
	}

	// Large spheres
	m_scene.push_back(std::make_unique<Sphere>(XMVECTORF32{ 0, 1, 0 }, 1.f, std::make_unique<DielectricTransparent>(XMVectorReplicate(16.f), 1.5f)));

	m_textures.push_back(std::make_unique<ConstTexture>(XMCOLOR{ 0.4f, 0.2f, 0.1f, 1.f }));
	m_scene.push_back(std::make_unique<Sphere>(XMVECTORF32{ -4, 1, 0 }, 1.f, std::make_unique<DielectricOpaque>(m_textures.back().get(), XMVectorReplicate(16.f))));

	m_textures.push_back(std::make_unique<ConstTexture>(XMCOLOR{ 0.7f, 0.6f, 0.5f, 1.f }));
	m_scene.push_back(std::make_unique<Sphere>(XMVECTORF32{ 4, 1, 0 }, 1.f, std::make_unique<Metal>(m_textures.back().get(), XM_Zero)));

	// Construct BVH
	m_bvh = std::make_unique<BvhNode>(m_scene.begin(), m_scene.end());

	// Sky
	m_textures.push_back(std::make_unique<ConstTexture>(XMCOLOR{ 0.85f, 0.91f, 0.98f, 1.f }));
	m_skyMaterial = std::make_unique<Emissive>(8000.f, m_textures.back().get());

	// Sun
	auto lightOcclusionTest = [this](const Ray& ray) -> bool
	{ 
		Payload dummy{};
		return m_bvh->Intersect(ray, dummy); 
	};
	m_lights.push_back(std::make_unique<DirectionalLight>(XMVECTORF32{ 1.f, 1.f, 1.f }, XMCOLOR{ 1.f, 0.97f, 0.88f, 1.f }, 40000.f, lightOcclusionTest));
}

std::vector<std::pair<Ray, int>> SpheresApp::GenerateRays() const
{
	std::vector<std::pair<Ray, int>> rays;
	rays.reserve(AppSettings::k_backbufferWidth * AppSettings::k_backbufferHeight);

	const auto xsize = static_cast<float>(AppSettings::k_backbufferWidth);
	const auto ysize = static_cast<float>(AppSettings::k_backbufferHeight);

	XMFLOAT2 jitterOffset = Random::HaltonSample2D(m_sampleCount, 2, 3);

	int rayId = 0;

	for (auto j = 0; j < AppSettings::k_backbufferHeight; ++j)
	{
		for (auto i = 0; i < AppSettings::k_backbufferWidth; ++i)
		{
			XMFLOAT2 uv;
			uv.x = static_cast<float>(i + jitterOffset.x) / xsize;
			uv.y = static_cast<float>(j + jitterOffset.y) / ysize;

			const XMFLOAT2 offset = Random::HaltonSampleDisk(m_sampleCount + i + j, 4, 5);

			const Ray ray = m_camera->GetRay(uv, offset);

			rays.push_back(std::make_pair(ray, rayId++));
		}
	}

	return rays;
}

size_t SpheresApp::DrawBitmap(HWND hWnd)
{
	using namespace DirectX;
	using namespace DirectX::PackedVector;

	++m_sampleCount;

	// Rays
	std::vector<std::pair<Ray, int>> rayBuffer = GenerateRays();

	// Exposure for the scene
	const float exposureAdjustment = std::pow(2, m_exposure);

	// Trace
	std::for_each(
		std::execution::par,
		rayBuffer.cbegin(), rayBuffer.cend(), 
		[this, exposureAdjustment](const std::pair<Ray, int>& r)
		{
			XMVECTOR& colorVec = m_backbufferHdr[r.second];
			colorVec += GetHitColor(r.first, 0) * exposureAdjustment;
		});

	// ACES tonemapping parameters
	static const float a = 2.51f;
	static const float b = 0.03f;
	static const float c = 2.43f;
	static const float d = 0.59f;
	static const float e = 0.14f;

	// Gamma
	static XMVECTORF32 invGamma{ 1 / 2.2f, 1 / 2.2f, 1 / 2.2f };

	std::transform(
		std::execution::par,
		m_backbufferHdr.cbegin(), m_backbufferHdr.cend(),
		m_backbufferLdr.begin(),
		[n = m_sampleCount](const XMVECTOR& hdrColor) -> XMCOLOR
		{
			XMVECTOR color = hdrColor / static_cast<float>(n);

			// Tonemap
			color = XMVectorSaturate((color*(a*color + XMVectorReplicate(b))) / (color*(c*color + XMVectorReplicate(d)) + XMVectorReplicate(e)));

			// Gamma correction
			color = XMVectorPow(color, invGamma);

			XMCOLOR outColor;
			XMStoreColor(&outColor, color);

			return outColor;
		});


	m_backbufferBitmap->CopyFromMemory(nullptr, m_backbufferLdr.data(), sizeof(m_backbufferLdr[0]) * AppSettings::k_backbufferWidth);

	m_renderTarget->DrawBitmap(m_backbufferBitmap.Get());

	return rayBuffer.size();
}

std::optional<Payload> SpheresApp::GetClosestIntersection(const Ray& ray) const
{
	Payload payload{};

	if (m_bvh->Intersect(ray, payload))
	{
		return payload;
	}
	else
	{
		return std::nullopt;
	}
}

XMVECTOR SpheresApp::GetHitColor(const Ray& ray, int depth) const
{
	if (auto hitInfo = GetClosestIntersection(ray))
	{
		const Payload& hit = hitInfo.value();

		XMVECTOR attenuation;
		Ray scatteredRay;
		const bool isScattered = hit.material->Scatter(ray, hit, attenuation, scatteredRay);
		const bool recurse = depth < AppSettings::k_recursionDepth && isScattered;

		return hit.material->Emit(hit) +
			hit.material->Shade(hit, m_lights, m_camera->GetOrigin()) +
			(recurse ? attenuation * GetHitColor(scatteredRay, depth + 1) : XM_Zero);
	}
	else
	{
		return m_skyMaterial->Emit(Payload{});
	}
}

void SpheresApp::DisplayStats(HWND hWnd, const size_t rayCount, const double timeElapsed) const
{
	static double totalTimeInSeconds = 0;
	totalTimeInSeconds += timeElapsed * std::pow(10, -6);

	const double mraysPerSecond = static_cast<double>(rayCount) / timeElapsed;

	std::wstring windowText = std::wstring(L"Demo") +
		L"\t | Mrays/s: " + std::to_wstring(mraysPerSecond) +
		L"\t | spp: " + std::to_wstring(m_sampleCount) + 
		L"\t | Time (seconds): " + std::to_wstring(totalTimeInSeconds);

	SetWindowText(hWnd, windowText.c_str());
}

int SpheresApp::GetBackBufferWidth() const
{
	return AppSettings::k_backbufferWidth;
}

int SpheresApp::GetBackBufferHeight() const
{
	return AppSettings::k_backbufferHeight;
}