#pragma once

#include "stdafx.h"

namespace AppSettings
{
	constexpr int k_backbufferWidth = 1280;
	constexpr int k_backbufferHeight = 720; 
	constexpr int k_recursionDepth = 50;
	constexpr float k_verticalFov = 25.f;
	constexpr float k_aperture = 0.4f;
	constexpr float k_aspectRatio = k_backbufferWidth / static_cast<float>(k_backbufferHeight);
}

class SpheresApp : public RayTracingApp
{
private:
	void OnInitialize(HWND hWnd) override;
	void OnRender(HWND hWnd) override;
	int GetBackBufferWidth() const override;
	int GetBackBufferHeight() const override;

	void InitScene();
	void InitCamera();

	size_t DrawBitmap(HWND hWnd);
	void DisplayStats(HWND hWnd, size_t rayCount, double timeElapsed) const;

	std::optional<Payload> GetClosestIntersection(const Ray& ray) const;
	XMVECTOR GetHitColor(const Ray& ray, int depth) const;

	std::vector<std::pair<Ray, int>> GenerateRays() const;

private:
	std::unique_ptr<Camera> m_camera;
	std::vector<std::unique_ptr<Hitable>> m_scene;
	std::vector<std::unique_ptr<Texture>> m_textures;
	std::vector<std::unique_ptr<Light>> m_lights;
	std::unique_ptr<BvhNode> m_bvh;
	std::unique_ptr<Material> m_skyMaterial;
	float m_exposure;
	size_t m_sampleCount = 0;
};
