#pragma once

#include "RayTracing.h"

namespace AppSettings
{
	static const std::wstring k_windowCaption = L"Pathtracing Demo";

	constexpr int k_backbufferWidth = 1280;
	constexpr int k_backbufferHeight = 720; 
	constexpr int k_samplesPerPixel = 100;
	constexpr int k_recursionDepth = 50;
	constexpr float k_verticalFov = 25.f;
	constexpr float k_aperture = 0.4f;
	constexpr float k_aspectRatio = k_backbufferWidth / static_cast<float>(k_backbufferHeight);
}

class Camera
{
public:
	Camera(XMVECTOR origin, XMVECTOR lookAt, float verticalFOV, float aspectRatio, float focalLength, float aperture);
	Ray GetRay(float u, float v) const;

private:
	DirectX::XMVECTOR m_origin;
	DirectX::XMVECTOR m_x;
	DirectX::XMVECTOR m_y;
	DirectX::XMVECTOR m_lowerLeft;
	float m_aperture;
	float m_focalLength;
};

class RayTracingApp
{
public:
	void OnInitialize(HWND hWnd);
	void OnRender(HWND hWnd);

private:
	void InitDirect2D(HWND hWnd) noexcept;
	void InitBuffers();
	void InitScene();

	size_t DrawBitmap(HWND hWnd);

	std::optional<Payload> GetClosestIntersection(const Ray& ray) const;
	XMVECTOR GetSceneColor(const Ray& ray, int depth) const;

	std::vector<std::pair<Ray,int>> GenerateRays() const;
	std::pair<float, float> GetJitterOffset() const;

	void DisplayStats(HWND hWnd, size_t rayCount, double timeElapsed) const;

private:
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	std::vector<DirectX::XMVECTOR> m_backbufferHdr;
	std::vector<DirectX::PackedVector::XMCOLOR> m_backbufferLdr;

	std::unique_ptr<Camera> m_camera;

	size_t m_sampleCount = 0;

	std::vector<Sphere> m_scene;
};
