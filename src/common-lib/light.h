#pragma once

#include "stdafx.h"
#include "ray-tracing.h"

class Light
{
public:
	virtual XMVECTOR Shade(const class Material* material, const Payload& payload, const XMVECTOR& viewOrigin) const = 0;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight(const XMVECTOR& dir, const XMCOLOR& color, const float luminance, std::function<bool(const Ray& ray)> lightOcclusionTest);
	XMVECTOR Shade(const class Material* material, const Payload& payload, const XMVECTOR& viewOrigin) const override;

private:
	XMVECTOR m_direction;
	XMVECTOR m_color;
	float m_luminance;
	std::function<bool(const Ray& ray)> IsOccluded;
};