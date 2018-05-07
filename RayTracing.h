#pragma once

using namespace DirectX;

__declspec(align(16))
struct Payload
{
	XMVECTOR t;
	XMVECTOR p;
	XMVECTOR normal;
	int materialIndex;
};

__declspec(align(16))
struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	Ray(const XMVECTOR& o, const XMVECTOR& d);
	XMVECTOR Evaluate(float t);
};

__declspec(align(16))
struct Sphere
{
	XMVECTOR center;
	float radius;
	int materialIndex;

	Sphere(const XMVECTOR& c, const float r, int matIndex);
	bool Intersect(const Ray& ray, const XMVECTOR tmin, const XMVECTOR tmax, Payload& payload) const;
};

class Material
{
public:
	virtual std::optional<Ray> Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation) = 0;
};

class Lambertian : public Material
{
public:
	Lambertian(float r, float g, float b);
	std::optional<Ray> Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation) override;

private:
	XMVECTOR m_albedo;
};

class Metal : public Material
{
public:
	Metal(float r, float g, float b);
	std::optional<Ray> Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation) override;

private:
	XMVECTOR m_albedo;
};

class Dielectric : public Material
{
public:
	Dielectric(float ior);
	std::optional<Ray> Scatter(const Ray& ray, const Payload& payload, XMVECTOR& outAttenuation) override;

private:
	XMVECTOR m_ior;
};