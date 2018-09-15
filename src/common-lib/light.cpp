#include "light.h"
#include "material.h"

DirectionalLight::DirectionalLight(const XMVECTOR& dir, const XMCOLOR& color, const float luminance, std::function<bool(const Ray& ray)> lightOcclusionTest) :
	m_luminance{luminance}, IsOccluded{std::move(lightOcclusionTest)}
{
	m_direction = XMVector3Normalize(dir);
	m_color = XMLoadColor(&color);
}

XMVECTOR DirectionalLight::Shade(const class Material* material, const Payload& payload, const XMVECTOR& viewOrigin) const
{
	Ray shadowRay{ payload.pos, m_direction };

	if (IsOccluded(shadowRay))
	{
		return XM_Zero;
	}
	else
	{
		XMVECTOR albedo = material->GetAlbedo(payload.uv);
		XMVECTOR f0 = material->GetReflectance(payload.uv);
		XMVECTOR smoothness = material->GetSmoothness(payload.uv);

		// Incoming light
		XMVECTOR nDotL = XMVectorSaturate(XMVector3Dot(payload.normal, m_direction));
		XMVECTOR radianceIn = m_luminance * m_color * nDotL;

		// Diffuse
		XMVECTOR diffuseBRDF = albedo;

		// Specular
		XMVECTOR viewDir = XMVector3Normalize(viewOrigin - payload.pos);
		XMVECTOR halfVector = XMVector3Normalize(m_direction + viewDir);
		XMVECTOR nDotH = XMVectorSaturate(XMVector3Dot(payload.normal, halfVector));
		XMVECTOR nDotV = XMVectorSaturate(XMVector3Dot(viewDir, payload.normal));
		XMVECTOR reflectance = f0 + (XM_One - f0) * XMVectorPow(XM_One - nDotV, XMVectorReplicate(5.f));
		XMVECTOR specularBRDF = reflectance * 0.125f * (smoothness + XMVectorReplicate(8.f)) * XMVectorPow(nDotH, smoothness);

		return radianceIn * (diffuseBRDF + specularBRDF);
	}
}