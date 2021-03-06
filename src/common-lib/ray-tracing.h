#pragma once

#include "stdafx.h"

__declspec(align(16))
struct Payload
{
	XMVECTOR t;
	XMVECTOR pos;
	XMVECTOR normal;
	XMFLOAT2 uv;
	class Material* material;
};

__declspec(align(16))
struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	Ray() = default;
	Ray(const XMVECTOR& o, const XMVECTOR& d) noexcept;
	XMVECTOR Evaluate(float t);
};

struct AABB
{
	BoundingBox m_box;

	AABB() = default;
	AABB(const XMFLOAT3& center, const XMFLOAT3& extents);
	bool Intersect(const Ray& ray) const;
};

struct Hitable
{
	virtual AABB GetAABB() const = 0;
	virtual bool Intersect(const Ray& ray, Payload& payload) const = 0;
};

struct BvhNode : public Hitable
{
	AABB m_aabb;
	std::unique_ptr<Hitable> m_left;
	std::unique_ptr<Hitable> m_right;

	using Iter = std::vector<std::unique_ptr<Hitable>>::iterator;
	BvhNode(Iter begin, Iter end);
	AABB GetAABB() const;
	bool Intersect(const Ray& ray, Payload& payload) const override;
};

struct Sphere : public Hitable
{
	__declspec(align(16)) XMVECTOR center;
	float radius;
	std::unique_ptr<class Material> material;

	Sphere(const XMVECTOR& c, const float r, std::unique_ptr<class Material>&& mat) noexcept;
	AABB GetAABB() const;
	bool Intersect(const Ray& ray, Payload& payload) const override;

private:
	XMFLOAT2 ComputeUV(const XMVECTOR& worldPos) const;
};