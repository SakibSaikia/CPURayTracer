#include "stdafx.h"

XMVECTOR GetRandomVectorInUnitSphere()
{
	static std::random_device device;
	static std::mt19937 generator(device());
	static std::uniform_real_distribution<float> uniformDist(-1.f, 1.f);

	XMVECTOR p;
	uint32_t check;

	do
	{
		p = XMVECTORF32{ uniformDist(generator), uniformDist(generator), uniformDist(generator) };

		XMVECTOR lengthSq = XMVector3LengthSq(p);

		XMVectorGreaterR(&check, lengthSq, XMVECTORF32{1.f, 1.f, 1.f});

	} while (XMComparisonAllTrue(check));

	return p;
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

		uint32_t check1, check2;
		XMVectorGreaterR(&check1, t, tmin);
		XMVectorGreaterR(&check2, tmax, t);

		if (XMComparisonAllTrue(check1) && XMComparisonAllTrue(check2))
		{
			payload.t = t;
			payload.p = XMVectorMultiplyAdd(t, ray.direction, ray.origin);
			payload.normal = (payload.p - center) / XMVectorReplicate(radius);
			payload.materialIndex = materialIndex;

			return true;
		}

		t = (-b + XMVectorSqrt(discriminant)) / a;

		XMVectorGreaterR(&check1, t, tmin);
		XMVectorGreaterR(&check2, tmax, t);

		if (XMComparisonAllTrue(check1) && XMComparisonAllTrue(check2))
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

bool Lambertian::Scatter(const Ray& ray, const Payload& hit, XMVECTOR& outAttenuation, Ray& outScatteredRay)
{
	XMVECTOR target = hit.p + hit.normal + GetRandomVectorInUnitSphere();
	outScatteredRay = { hit.p, target - hit.p };
	outAttenuation = m_albedo;

	return true;
}