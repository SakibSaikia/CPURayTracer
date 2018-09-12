#pragma once

#include "stdafx.h"
#include "ray-tracing.h"
#include "texture.h"

class Material
{
public:
	virtual bool AbsorbAndScatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const = 0;
	virtual XMVECTOR Emit(XMFLOAT2 uv) const = 0;
};

class Metal : public Material
{
public:
	Metal(const Texture* reflectance);
	bool AbsorbAndScatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(XMFLOAT2 uv) const override { return XMVectorZero(); }

private:
	const Texture* m_reflectance;
};

class DielectricOpaque : public Material
{
public:
	DielectricOpaque(const Texture* albedo, const float ior);
	bool AbsorbAndScatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(XMFLOAT2 uv) const override { return XMVectorZero(); }

private:
	const Texture* m_albedo;
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
	mutable std::atomic<uint64_t> m_reflectionProbabilitySampleIndex = 0u;
};

class DielectricTransparent : public Material
{
public:
	DielectricTransparent(float ior);
	bool AbsorbAndScatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(XMFLOAT2 uv) const override { return XMVectorZero(); }

private:
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
};

class Emissive : public Material
{
public:
	Emissive(const float luminance, const Texture* color);
	bool AbsorbAndScatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override { return false; }
	XMVECTOR Emit(XMFLOAT2 uv) const override;

private:
	const Texture* m_color;
	float m_luminance;
};