#include "stdafx.h"

void RandGenerator::Init()
{
	std::random_device device;
	std::mt19937 generator(device());
	std::uniform_real_distribution<float> uniformDist(-1.f, 1.f);

	XMVECTOR p, lengthSq;
	static const XMVECTORF32 one { 1.f, 1.f, 1.f };

	for (auto n = 0; n < k_randCount; ++n)
	{
		do
		{
			p = XMVECTORF32{ uniformDist(generator), uniformDist(generator), uniformDist(generator) };

			lengthSq = XMVector3LengthSq(p);

		} while (XMVector3Greater(lengthSq, one));

		m_unitSphereVectorCache[n] = p;
	}

	m_startTime = std::chrono::high_resolution_clock::now();
}

XMVECTOR RandGenerator::VectorInUnitSphere()
{
	return m_unitSphereVectorCache[Xorshift()];
}

XMVECTOR RandGenerator::VectorInUnitDisk()
{
	XMVECTOR v = m_unitSphereVectorCache[Xorshift()];
	return XMVectorSetZ(v, 0.f);
}

int RandGenerator::Xorshift()
{
	// Xorshift PRNG 
	// https://en.wikipedia.org/wiki/Xorshift

	auto timeNow = std::chrono::high_resolution_clock::now();
	std::chrono::duration<uint64_t, std::nano> duration = (timeNow - m_startTime);
	uint64_t x = duration.count();
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;

	return (x % k_randCount);
}

Ray::Ray(const XMVECTOR& o, const XMVECTOR& d) :
	origin{ o }, direction{ d } 
{
}

XMVECTOR Ray::Evaluate(float t)
{
	// origin + t * direction
	return XMVectorMultiplyAdd(direction, XMVectorReplicate(t), origin);
}

Sphere::Sphere(const XMVECTOR& c, const float r, const int matIndex) :
	center{ c }, radius{ r }, materialIndex{ matIndex }
{
}


bool Sphere::Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload) const
{
	XMVECTOR oc = ray.origin - center;

	XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	XMVECTOR b = XMVector3Dot(oc, ray.direction);
	XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	XMVECTOR discriminant = b*b - a*c;

	if (XMVectorGetX(discriminant) > 0.f)
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.materialIndex = materialIndex;

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.materialIndex = materialIndex;

			return true;
		}
	}

	return false;
}

Lambertian::Lambertian(const float r, const float g, const float b)
{
	m_albedo = XMVectorSet(r, g, b, 1.f);
}

std::optional<Ray> Lambertian::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation)
{
	outAttenuation = m_albedo;

	XMVECTOR target = hit.p + hit.normal + RandGenerator::VectorInUnitSphere();
	Ray scatteredRay = { hit.p, target - hit.p };

	return scatteredRay;
}

Metal::Metal(const float r, const float g, const float b)
{
	m_albedo = XMVectorSet(r, g, b, 1.f);
}

std::optional<Ray> Metal::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation)
{
	outAttenuation = m_albedo;

	XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));

	if (XMVector3Greater(XMVector3Dot(reflectDir, hit.normal), XMVectorZero()))
	{
		Ray scatteredRay = { hit.p, reflectDir };
		return scatteredRay;
	}
	else
	{
		return std::nullopt;
	}
}

Dielectric::Dielectric(const float ior)
{
	m_ior = XMVectorReplicate(ior);
}

std::optional<Ray> Dielectric::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation)
{
	// Attenuation of 1 for glass
	outAttenuation = { 1.f, 1.f, 1.f };

	XMVECTOR outwardNormal;
	XMVECTOR niOverNt;
	XMVECTOR cosineIncidentAngle;
	XMVECTOR reflectionProbability;

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

	auto refractionResult = Refract(ray.direction, outwardNormal, niOverNt);

	if (refractionResult)
	{
		// Valid refraction, but can still be reflected based on fresnel term
		reflectionProbability = XMFresnelTerm(cosineIncidentAngle, m_ior);
	}
	else
	{
		// Total Internal Reflection
		reflectionProbability = XMVectorReplicate(1.f);
	}

	static std::random_device device;
	static std::mt19937 generator(device());
	static std::uniform_real_distribution<float> uniformDist(0.f, 1.f);

	XMVECTOR rand = XMVectorReplicate(uniformDist(generator));

	if (XMVector3Greater(reflectionProbability, rand))
	{
		XMVECTOR reflectDir = XMVector3Normalize(XMVector3Reflect(ray.direction, hit.normal));
		return Ray{ hit.p, reflectDir };
	}
	else
	{
		return Ray{ hit.p, refractionResult.value() };
	}
}

std::optional<XMVECTOR> Dielectric::Refract(const XMVECTOR& v, const XMVECTOR& n, const XMVECTOR niOverNt)
{
	XMVECTOR nDotV = XMVector3Dot(v, n);
	static const XMVECTORF32 one{ 1.f, 1.f, 1.f };

	XMVECTOR discriminant = one - niOverNt * niOverNt * (one - nDotV * nDotV);

	if (XMVector3Greater(discriminant, XMVectorZero()))
	{
		return niOverNt * (v - n * nDotV) - n * XMVectorSqrtEst(discriminant);
	}
	else
	{
		return std::nullopt;
	}
}