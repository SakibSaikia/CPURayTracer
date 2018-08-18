#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

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
	uint64_t m_sampleIndex = 0u;
};