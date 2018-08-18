#pragma once

#include "stdafx.h"

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
	uint64_t m_sampleIndex = 0u;
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
	Ray GetRay(XMFLOAT2 uv, XMFLOAT2 offset) const;

private:
	DirectX::XMVECTOR m_origin;
	DirectX::XMVECTOR m_x;
	DirectX::XMVECTOR m_y;
	DirectX::XMVECTOR m_lowerLeft;
	float m_aperture;
	float m_focalLength;
};