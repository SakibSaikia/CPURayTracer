#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

class Material
{
public:
	virtual bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const = 0;
	virtual XMVECTOR Emit() const = 0;
};

class Dielectric : public Material
{
public:
	Dielectric(const XMCOLOR& albedo);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit() const override { return XMVectorZero(); }

private:
	XMVECTOR m_albedo;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
	mutable std::atomic<uint64_t> m_reflectionProbabilitySampleIndex = 0u;
};

class Metal : public Material
{
public:
	Metal(const XMCOLOR& reflectance);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit() const override { return XMVectorZero(); }

private:
	XMVECTOR m_reflectance;
};

class Transparent : public Material
{
public:
	Transparent(float ior);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit() const override { return XMVectorZero(); }

private:
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
};

class Emissive : public Material
{
public:
	Emissive(float lux, const XMCOLOR& color);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override { return false; }
	XMVECTOR Emit() const override;

private:
	XMVECTOR m_luminance;
};