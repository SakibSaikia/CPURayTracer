#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

class Material
{
public:
	virtual bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const = 0;
};

class Lambertian : public Material
{
public:
	Lambertian(float r, float g, float b);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;

private:
	XMVECTOR m_albedo;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
};

class Metal : public Material
{
public:
	Metal(float r, float g, float b);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;

private:
	XMVECTOR m_albedo;
};

class Dielectric : public Material
{
public:
	Dielectric(float ior);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;

private:
	bool Refract(const XMVECTOR& v, const XMVECTOR& n, const XMVECTOR niOverNt, XMVECTOR& outDir) const;

private:
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
};