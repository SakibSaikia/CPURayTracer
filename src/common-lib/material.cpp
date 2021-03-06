#include "material.h"
#include "quasi-random.h"

XMVECTOR Material::Shade(const Payload& payload, const std::vector<std::unique_ptr<Light>>& lights, const XMVECTOR& viewOrigin) const
{
	XMVECTOR directLighting = XM_Zero;
	for (const auto& light : lights)
	{
		directLighting += light->Shade(this, payload, viewOrigin);
	}

	return directLighting;
}

DielectricOpaque::DielectricOpaque(const Texture* albedo, const XMVECTOR& smoothness) :
	m_albedo{ albedo }, m_smoothness{ smoothness }
{
}

bool DielectricOpaque::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const
{
	if (XMVector3Greater(XMVector3Dot(-ray.direction, hit.normal), XM_Zero))
	{
		// Use ray direction to calculate incident angle. This is same as view direction for primary rays.
		XMVECTOR f0 = GetReflectance(hit.uv);
		XMVECTOR nDotV = XMVectorSaturate(XMVector3Dot(-ray.direction, hit.normal));
		XMVECTOR reflectance = f0 + (XM_One - f0) * XMVectorPow(XM_One - nDotV, XMVectorReplicate(5.f));
		
		const XMVECTOR rand = XMVectorReplicate(Random::HaltonSample(m_reflectionProbabilitySampleIndex++, 3));
		bool bReflect = XMVector3Greater(reflectance, rand);

		if (bReflect)
		{
			outAttenuation = XM_One;
			const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
			outRay = { hit.pos, reflectDir };
			return true;
		}
		else
		{
			outAttenuation = m_albedo->Evaluate(hit.uv);

			// Random sample direction in unit hemisphere
			XMFLOAT3 dir = Random::HaltonSampleHemisphere(m_sampleIndex++, 5, 7);

			// Orthonormal basis about hit normal
			XMVECTOR b3 = hit.normal;
			XMFLOAT3 temp;
			XMStoreFloat3(&temp, b3);
			XMVECTOR up = std::abs(temp.x) < 0.5f ? XMVECTORF32{ 1.0f, 0.0f, 0.0f } : XMVECTORF32{ 0.0f, 1.0f, 0.0f };
			XMVECTOR b1 = XMVector3Cross(up, b3);
			XMVECTOR b2 = XMVector3Cross(b3, b1);

			// Project sample direction into ortho basis
			const XMVECTOR scatterDir = dir.x * b1 + dir.y * b2 + dir.z * b3;
			outRay = { hit.pos, XMVector3Normalize(scatterDir) };

			return true;
		}
	}
	else
	{
		return false;
	}
}

Metal::Metal(const Texture* reflectance, const XMVECTOR& smoothness) :
	m_reflectance{ reflectance }, m_smoothness{ smoothness }
{
}

bool Metal::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const
{
	if (XMVector3Greater(XMVector3Dot(-ray.direction, hit.normal), XM_Zero))
	{
		// Use ray direction to calculate incident angle. This is same as view direction for primary rays.
		XMVECTOR f0 = GetReflectance(hit.uv);
		XMVECTOR nDotV = XMVectorSaturate(XMVector3Dot(-ray.direction, hit.normal));
		XMVECTOR reflectance = f0 + (XM_One - f0) * XMVectorPow(XM_One - nDotV, XMVectorReplicate(5.f));

		uint32_t bReflect;
		const XMVECTOR rand = XMVectorReplicate(Random::HaltonSample(m_reflectionProbabilitySampleIndex++, 3));
		XMVectorGreaterR(&bReflect, reflectance, rand);

		if (XMComparisonAnyTrue(bReflect))
		{
			outAttenuation = m_reflectance->Evaluate(hit.uv);

			const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
			outRay = { hit.pos, reflectDir };

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

DielectricTransparent::DielectricTransparent(const XMVECTOR& smoothness, const float ior) :
	m_smoothness{ smoothness }
{
	m_ior = XMVectorReplicate(ior);
}

bool DielectricTransparent::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const
{
	// Attenuation of 1 for glass (no absorption)
	outAttenuation = { 1.f, 1.f, 1.f };

	XMVECTOR outwardNormal{};
	XMVECTOR niOverNt{};
	XMVECTOR cosineIncidentAngle{};
	XMVECTOR reflectionProbability{};

	if (XMVector3Greater(XMVector3Dot(ray.direction, hit.normal), XM_Zero))
	{
		// Air-to-medium
		outwardNormal = -hit.normal;
		niOverNt = m_ior;
		cosineIncidentAngle = XMVector3Dot(ray.direction, hit.normal);
	}
	else
	{
		// Medium-to-air
		outwardNormal = hit.normal;
		niOverNt = XMVectorReciprocalEst(m_ior);
		cosineIncidentAngle = XMVector3Dot(ray.direction, -hit.normal);
	}

	// Returns < 0.0f, 0.0f, 0.0f, undefined > if result is a total internal reflection
	XMVECTOR refractDir = XMVector3RefractV(ray.direction, outwardNormal, niOverNt);
	bool canRefract = XMVector3NotEqual(refractDir, XM_Zero);

	if (canRefract)
	{
		// Valid refraction, but can still be reflected based on fresnel term
		reflectionProbability = XMFresnelTerm(cosineIncidentAngle, m_ior);
	}
	else
	{
		// Total Internal Reflection
		reflectionProbability = XM_One;
	}

	const XMVECTOR rand = XMVectorReplicate(Random::HaltonSample(m_sampleIndex++, 7));

	if (XMVector3Greater(reflectionProbability, rand))
	{
		const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
		outRay = { hit.pos, reflectDir };
		return true;
	}
	else
	{
		outRay = { hit.pos, XMVector3Normalize(refractDir) };
		return true;
	}
}

Emissive::Emissive(const float luminance, const Texture* color) :
	m_luminance{ luminance }, 
	m_color { color }
{
}

XMVECTOR Emissive::Emit(const Payload& payload) const
{
	return m_luminance * m_color->Evaluate(payload.uv);
}