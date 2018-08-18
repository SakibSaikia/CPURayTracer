#include "ray-tracing.h"
#include "quasi-random.h"

Ray::Ray(const XMVECTOR& o, const XMVECTOR& d) noexcept :
	origin{ o }, direction{ d } 
{
}

XMVECTOR Ray::Evaluate(float t)
{
	// origin + t * direction
	return XMVectorMultiplyAdd(direction, XMVectorReplicate(t), origin);
}

Sphere::Sphere(const XMVECTOR& c, const float r, std::unique_ptr<Material>&& mat) noexcept :
	center{ c }, radius{ r }, material{ std::move(mat) }
{
}


bool Sphere::Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload) const
{
	const XMVECTOR oc = ray.origin - center;

	const XMVECTOR a = XMVector3Dot(ray.direction, ray.direction);
	const XMVECTOR b = XMVector3Dot(oc, ray.direction);
	const XMVECTOR c = XMVector3Dot(oc, oc) - XMVectorReplicate(radius * radius);

	const XMVECTOR discriminant = b*b - a*c;

	if (XMVector3Greater(discriminant, XMVectorZero()))
	{
		XMVECTOR t = (-b - XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.material = material.get();

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;

		if (XMVector3Greater(t, tmin) && XMVector3Less(t, tmax))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.material = material.get();

			return true;
		}
	}

	return false;
}

Lambertian::Lambertian(const float r, const float g, const float b)
{
	m_albedo = XMVectorSet(r, g, b, 1.f);
}

bool Lambertian::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay)
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

bool Metal::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay)
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

bool Dielectric::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outRay)
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

	static std::random_device device;
	static std::mt19937 generator(device());
	static std::uniform_real_distribution<float> uniformDist(0.f, 1.f);

	const XMVECTOR rand = XMVectorReplicate(uniformDist(generator));

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

bool Dielectric::Refract(const XMVECTOR& v, const XMVECTOR& n, const XMVECTOR niOverNt, XMVECTOR& outDir)
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

Camera::Camera(
	const XMVECTOR origin,
	const XMVECTOR lookAt,
	const float verticalFOV,
	const float aspectRatio,
	const float focalLength,
	const float aperture) :
	m_origin{ origin },
	m_aperture{ aperture },
	m_focalLength{ focalLength }
{
	const float theta = verticalFOV * XM_PI / 180.f;
	const float halfHeight = std::tan(theta / 2.f);
	const float halfWidth = aspectRatio * halfHeight;

	const XMVECTORF32 up{ 0.f, 1.f, 0.f };
	const XMVECTOR w = XMVector3Normalize(origin - lookAt);
	const XMVECTOR u = XMVector3Normalize(XMVector3Cross(up, w));
	const XMVECTOR v = XMVector3Cross(w, u);

	m_lowerLeft = origin - halfWidth * u - halfHeight * v - w;
	m_x = 2 * halfWidth * u;
	m_y = 2 * halfHeight * v;
}

Ray Camera::GetRay(XMFLOAT2 uv, XMFLOAT2 offset) const
{
	// Use primary ray to determine focal point
	const XMVECTOR p = m_lowerLeft + uv.x * m_x + uv.y * m_y;
	const XMVECTOR focalPoint = m_origin + m_focalLength * XMVector3Normalize(p - m_origin);

	// Secondary ray used for tracing
	XMFLOAT2 rd;
	rd.x = 0.5f * m_aperture * offset.x;
	rd.y = 0.5f * m_aperture * offset.y;
	const XMVECTOR origin = m_origin + rd.x * m_x + rd.y * m_y;

	return Ray{ origin, XMVector3Normalize(focalPoint - origin) };
}