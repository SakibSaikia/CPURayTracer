#include "material.h"
#include "quasi-random.h"

Lambertian::Lambertian(const float r, const float g, const float b)
{
	m_albedo = XMVectorSet(r, g, b, 1.f);
}

bool Lambertian::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const 
{
	outAttenuation = m_albedo;

	// Random sample direction in unit hemisphere
	XMFLOAT3 dir = Random::HaltonSampleHemisphere(m_sampleIndex++, 5, 7);

	// Orthonormal basis about hit normal
	XMVECTOR b3 = XMVector3Normalize(hit.normal);
	XMFLOAT3 temp;
	XMStoreFloat3(&temp, b3);
	XMVECTOR up = std::abs(temp.x) < 0.5f ? XMVECTORF32{ 1.0f, 0.0f, 0.0f } : XMVECTORF32{ 0.0f, 1.0f, 0.0f };
	XMVECTOR b1 = XMVector3Cross(up, b3);
	XMVECTOR b2 = XMVector3Cross(b3, b1);

	// Project sample direction into ortho basis
	const XMVECTOR scatterDir = dir.x * b1 + dir.y * b2 + dir.z * b3;
	outRay = { hit.p, scatterDir };

	return true;
}

Metal::Metal(const float r, const float g, const float b)
{
	m_albedo = XMVectorSet(r, g, b, 1.f);
}

bool Metal::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const 
{
	outAttenuation = m_albedo;

	const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));

	if (XMVector3Greater(XMVector3Dot(reflectDir, hit.normal), XMVectorZero()))
	{
		outRay = { hit.p, reflectDir };
		return true;
	}
	else
	{
		return false;
	}
}

Dielectric::Dielectric(const float ior)
{
	m_ior = XMVectorReplicate(ior);
}

bool Dielectric::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const
{
	// Attenuation of 1 for glass
	outAttenuation = { 1.f, 1.f, 1.f };

	XMVECTOR outwardNormal{};
	XMVECTOR niOverNt{};
	XMVECTOR cosineIncidentAngle{};
	XMVECTOR reflectionProbability{};

	if (XMVector3Greater(XMVector3Dot(ray.direction, hit.normal), XMVectorZero()))
	{
		// Air-to-medium
		outwardNormal = -hit.normal;
		niOverNt = m_ior;
		cosineIncidentAngle = m_ior * XMVector3Dot(ray.direction, hit.normal);
	}
	else
	{
		// Medium-to-air
		outwardNormal = hit.normal;
		niOverNt = XMVectorReciprocalEst(m_ior);
		cosineIncidentAngle = -XMVector3Dot(ray.direction, hit.normal);
	}

	XMVECTOR refractDir;
	const bool canRefract = Refract(ray.direction, outwardNormal, niOverNt, refractDir);

	if (canRefract)
	{
		// Valid refraction, but can still be reflected based on fresnel term
		reflectionProbability = XMFresnelTerm(cosineIncidentAngle, m_ior);
	}
	else
	{
		// Total Internal Reflection
		reflectionProbability = XMVectorReplicate(1.f);
	}

	const XMVECTOR rand = XMVectorReplicate(Random::HaltonSample(m_sampleIndex++, 7));

	if (XMVector3Greater(reflectionProbability, rand))
	{
		const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
		outRay = { hit.p, reflectDir };
		return true;
	}
	else
	{
		outRay = { hit.p, refractDir };
		return true;
	}
}

bool Dielectric::Refract(const XMVECTOR& v, const XMVECTOR& n, const XMVECTOR niOverNt, XMVECTOR& outDir) const
{
	const XMVECTOR nDotV = XMVector3Dot(v, n);
	static const XMVECTORF32 one{ 1.f, 1.f, 1.f };

	const XMVECTOR discriminant = one - niOverNt * niOverNt * (one - nDotV * nDotV);

	if (XMVector3Greater(discriminant, XMVectorZero()))
	{
		outDir = niOverNt * (v - n * nDotV) - n * XMVectorSqrtEst(discriminant);
		return true;
	}
	else
	{
		return false;
	}
}