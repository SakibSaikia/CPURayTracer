#pragma once

#include "RayTracing.h"

namespace AppSettings
{
	static const std::wstring k_windowCaption = L"Pathtracing Demo";

	constexpr int k_backbufferWidth = 1280;
	constexpr int k_backbufferHeight = 720; 
	constexpr int k_samplesPerPixel = 100;
}

class Camera
{
public:
	Camera();
	Ray GetRay(float u, float v) const;

private:
	DirectX::XMVECTOR m_origin;
	DirectX::XMVECTOR m_x;
	DirectX::XMVECTOR m_y;
	DirectX::XMVECTOR m_lowerLeft;
};

class RayTracingApp
{
public:
	void OnInitialize(HWND hWnd);
	void OnRender(HWND hWnd);

private:
	void InitDirect2D(HWND hWnd);
	void InitBuffers();

	size_t DrawBitmap(HWND hWnd);

	std::optional<Payload> GetClosestIntersection(const Ray& ray) const;
	XMVECTOR GetSceneColor(const Ray& ray) const;

	std::vector<Ray> GenerateRays() const;
	std::pair<float, float> GetJitterOffset() const;

	void DisplayStats(HWND hWnd, size_t rayCount, double timeElapsed) const;

private:
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	std::vector<DirectX::XMVECTOR> m_backbufferHdr;
	std::vector<DirectX::PackedVector::XMCOLOR> m_backbufferLdr;

	Camera m_camera;

	size_t m_sampleCount = 0;

	const std::array<std::unique_ptr<Material>, 4> m_materials = {
		std::make_unique<Lambertian>(0.8f, 0.3f, 0.3f),
		std::make_unique<Lambertian>(0.8f, 0.8f, 0.f),
		std::make_unique<Metal>(0.8f, 0.6f, 0.2f),
		std::make_unique<Metal>(0.8f, 0.8f, 0.8f)
	};

	const std::array<Sphere, 4> m_scene = {
		Sphere{ XMVECTORF32{ 0.f, 0.f, -1.f },		0.5f,	0 },
		Sphere{ XMVECTORF32{ 0.f, -100.5f, -1.f },	100.f,	1 },
		Sphere{ XMVECTORF32{ 1.f, 0.f, -1.f },		0.5f,	2 },
		Sphere{ XMVECTORF32{ -1.f, 0.f, -1.f },		0.5f,	3 }
	};
};
