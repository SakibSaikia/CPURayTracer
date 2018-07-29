#pragma once

#define WIN32_LEAN_AND_MEAN
#define _SILENCE_PARALLEL_ALGORITHMS_EXPERIMENTAL_WARNING

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <execution>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <windows.h>
#include <d2d1.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <wrl.h>

using namespace DirectX;

__declspec(align(16))
struct Payload
{
	XMVECTOR t;
	XMVECTOR p;
	XMVECTOR normal;
	class Material* material;
};

__declspec(align(16))
struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	Ray() = default;
	Ray(const XMVECTOR& o, const XMVECTOR& d) noexcept;
	XMVECTOR Evaluate(float t);
};

__declspec(align(16))
struct Sphere
{
	XMVECTOR center;
	float radius;
	std::unique_ptr<class Material> material;

	Sphere(const XMVECTOR& c, const float r, std::unique_ptr<class Material>&& mat) noexcept;
	bool Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload) const;
};

class RandGenerator
{
public:
	static void Init();
	static XMVECTOR VectorInUnitSphere() noexcept;
	static XMVECTOR VectorInUnitDisk() noexcept;

private:
	static int Xorshift() noexcept;

private:
	static const uint64_t k_randCount = 4096;
	inline static std::array<XMVECTOR, k_randCount> m_unitSphereVectorCache;
	inline static std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

class Material
{
public:
	virtual bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) = 0;
};

class Lambertian : public Material
{
public:
	Lambertian(float r, float g, float b);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) override;

private:
	XMVECTOR m_albedo;
};

class Metal : public Material
{
public:
	Metal(float r, float g, float b);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) override;

private:
	XMVECTOR m_albedo;
};

class Dielectric : public Material
{
public:
	Dielectric(float ior);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) override;

private:
	bool Refract(const XMVECTOR& v, const XMVECTOR& n, const XMVECTOR niOverNt, XMVECTOR& outDir);

private:
	XMVECTOR m_ior;
};

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