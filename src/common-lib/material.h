#pragma once

#include "stdafx.h"
#include "ray-tracing.h"
#include "texture.h"
#include "light.h"

class Material
{
public:
	virtual bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const = 0;
	virtual XMVECTOR Shade(const Payload& payload, const std::vector<std::unique_ptr<Light>>& lights, const XMVECTOR& viewOrigin) const;
	virtual XMVECTOR Emit(const Payload& payload) const = 0;

	// Property getters
	virtual XMVECTOR GetAlbedo(XMFLOAT2 uv) const = 0;
	virtual XMVECTOR GetReflectance(XMFLOAT2 uv) const = 0;
	virtual XMVECTOR GetSmoothness(XMFLOAT2 uv) const = 0;
};

class Metal : public Material
{
public:
	Metal(const Texture* reflectance, const XMVECTOR& smoothness);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(const Payload& payload) const override { return XM_Zero; }
	XMVECTOR GetAlbedo(XMFLOAT2 uv) const override { return XM_Zero; } // all refracted light gets absorbed
	XMVECTOR GetReflectance(XMFLOAT2 uv) const override { return m_reflectance->Evaluate(uv); }
	XMVECTOR GetSmoothness(XMFLOAT2 uv) const override { return m_smoothness; }

private:
	const Texture* m_reflectance;
	XMVECTOR m_smoothness;
};

class DielectricOpaque : public Material
{
public:
	DielectricOpaque(const Texture* albedo, const XMVECTOR& smoothness, const float ior);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(const Payload& payload) const override { return XM_Zero; }
	XMVECTOR GetAlbedo(XMFLOAT2 uv) const override { return m_albedo->Evaluate(uv); }
	XMVECTOR GetReflectance(XMFLOAT2 uv) const override { return XMVECTORF32{ 0.04f, 0.04f, 0.04f, 1.f }; } // 4% reflectance for dielectrics
	XMVECTOR GetSmoothness(XMFLOAT2 uv) const override { return m_smoothness; }

private:
	const Texture* m_albedo;
	XMVECTOR m_smoothness;
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
	mutable std::atomic<uint64_t> m_reflectionProbabilitySampleIndex = 0u;
};

class DielectricTransparent : public Material
{
public:
	DielectricTransparent(const XMVECTOR& smoothness, float ior);
	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override;
	XMVECTOR Emit(const Payload& payload) const override { return XM_Zero; }
	XMVECTOR GetAlbedo(XMFLOAT2 uv) const override { return XM_Zero; } // refracted light gets transmitted
	XMVECTOR GetReflectance(XMFLOAT2 uv) const override { return XMVECTORF32{ 0.04f, 0.04f, 0.04f, 1.f }; } // 4% reflectance for dielectrics
	XMVECTOR GetSmoothness(XMFLOAT2 uv) const override { return m_smoothness; }

private:
	XMVECTOR m_smoothness;
	XMVECTOR m_ior;
	mutable std::atomic<uint64_t> m_sampleIndex = 0u;
};

class Emissive : public Material
{
public:
	Emissive(const float luminance, const Texture* color);
	XMVECTOR Emit(const Payload& payload) const override;

	bool Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation, Ray& outRay) const override { return false; }
	XMVECTOR GetAlbedo(XMFLOAT2 uv) const override { return XM_Zero; }
	XMVECTOR GetReflectance(XMFLOAT2 uv) const override { return XM_Zero; }
	XMVECTOR GetSmoothness(XMFLOAT2 uv) const override { return XM_Zero; }

private:
	const Texture* m_color;
	float m_luminance;
};