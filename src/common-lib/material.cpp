#include "material.h"
#include "quasi-random.h"

Dielectric::Dielectric(const XMCOLOR& albedo, const float ior)
{
	m_albedo = XMLoadColor(&albedo);
	m_ior = XMVectorReplicate(ior);
}

bool Dielectric::AbsorbAndScatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const 
{
	bool canReflect = XMVector3Greater(XMVector3Dot(-ray.direction, hit.normal), XMVectorZero());
	bool willReflect = false;

	if (canReflect)
	{
		XMVECTOR cosineIncidentAngle = XMVector3Dot(XMVector3Normalize(-ray.direction), XMVector3Normalize(hit.normal));
		XMVECTOR reflectionProbability = XMFresnelTerm(cosineIncidentAngle, XMVectorReplicate(1.3));
		const XMVECTOR rand = XMVectorReplicate(Random::HaltonSample(m_reflectionProbabilitySampleIndex++, 3));

		willReflect = XMVector3Greater(reflectionProbability, rand);
	}

	if (willReflect)
	{
		outAttenuation = XMVectorReplicate(1.f);
		const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
		outRay = { hit.p, reflectDir };
		return true;
	}
	else
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
		outRay = { hit.p, XMVector3Normalize(scatterDir) };

		return true;
	}
}

Metal::Metal(const XMCOLOR& reflectance)
{
	m_reflectance = XMLoadColor(&reflectance);
}

bool Metal::AbsorbAndScatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const 
{
	if (XMVector3Greater(XMVector3Dot(-ray.direction, hit.normal), XMVectorZero()))
	{
		outAttenuation = m_reflectance;

		const XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
		outRay = { hit.p, reflectDir };

		return true;
	}
	else
	{
		return false;
	}
}

Transparent::Transparent(const float ior)
{
	m_ior = XMVectorReplicate(ior);
}

bool Transparent::AbsorbAndScatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay) const
{
	// Attenuation of 1 for glass (no absorption)
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
	bool canRefract = XMVector3NotEqual(refractDir, XMVectorZero());

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
		outRay = { hit.p, XMVector3Normalize(refractDir) };
		return true;
	}
}

Emissive::Emissive(float lux, const XMCOLOR& color)
{
	XMVECTOR vColor = XMLoadColor(&color);
	XMVECTOR vLux = XMVectorReplicate(lux);

	m_luminance = vColor * vLux;
}

XMVECTOR Emissive::Emit() const
{
	return m_luminance;
}