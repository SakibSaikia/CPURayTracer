#pragma once

#include "RayTracing.h"

namespace AppSettings
{
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
	void DrawBitmap(HWND hWnd);
	std::vector<Ray> GenerateRays() const;
	std::pair<float, float> GetJitterOffset() const;

private:
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	std::vector<DirectX::XMVECTOR> m_backbufferHdr;
	std::vector<DirectX::PackedVector::XMCOLOR> m_backbufferLdr;

	Camera m_camera;

	size_t m_sampleCount = 0;
};
