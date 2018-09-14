#include "light.h"
#include "material.h"

DirectionalLight::DirectionalLight(const XMVECTOR& dir, const XMCOLOR& color, const float luminance, std::function<bool(const Ray& ray)> lightOcclusionTest) :
	m_luminance{luminance}, IsOccluded{std::move(lightOcclusionTest)}
{
	m_direction = XMVector3Normalize(dir);
	m_color = XMLoadColor(&color);
}

XMVECTOR DirectionalLight::Shade(const class Material* material, const Payload& payload) const
{
	Ray shadowRay{ payload.pos, m_direction };

	if (IsOccluded(shadowRay))
	{
		return XMVectorZero();
	}
	else
	{
		XMVECTOR albedo = material->GetAlbedo(payload.uv);
		XMVECTOR nDotL = XMVectorSaturate(XMVector3Dot(payload.normal, m_direction));
		return m_luminance * m_color * nDotL * albedo;
	}
}