#pragma once

#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include "ray.hpp"

namespace rt {

/**
	Axis-aligned bounding box
*/
class aabb : public ray_intersectable
{
public:
	aabb() = default;

	aabb(const glm::vec3 &mi, const glm::vec3 &ma) :
		min(mi),
		max(ma),
		size(max - min),
		half_size(size / 2.f),
		center((max + min) / 2.f)
	{}

	aabb(const aabb &lhs, const aabb &rhs) :
		min(glm::min(lhs.min, rhs.min)),
		max(glm::max(lhs.max, rhs.max)),
		size(max - min),
		half_size(size / 2.f),
		center((max + min) / 2.f)
	{}

	inline bool check_aabb_overlap(const aabb &rhs) const;
	inline bool check_point_inside(const glm::vec3 &p, float epsilon = 0.f) const;
	inline bool ray_intersect(const ray &r, ray_intersection &hit) const override;
	inline bool check_ray_intersect(const ray &r) const;
	inline float ray_intersection_distance(const ray &r) const;

	const glm::vec3 &get_min() const
	{
		return min;
	}

	const glm::vec3 &get_max() const
	{
		return max;
	}

	const glm::vec3 &get_size() const
	{
		return size;
	}

	const glm::vec3 &get_half_size() const
	{
		return half_size;
	}

	const glm::vec3 &get_center() const
	{
		return center;
	}

	const float get_surface_area() const
	{
		return 2.f * (size.x * size.y + size.x * size.z + size.z * size.y);
	}

private:
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 size;
	glm::vec3 half_size;
	glm::vec3 center;
};

/**
	Returns true if two AABBs overlap
*/
inline bool aabb::check_aabb_overlap(const aabb &rhs) const
{
	return	   min.x < rhs.max.x && max.x > rhs.min.x
			&& min.y < rhs.max.y && max.y > rhs.min.y
			&& min.z < rhs.max.z && max.z > rhs.min.z;
}


/**
	Checks whether a point is inside the box. Includes points on the faces
*/
inline bool aabb::check_point_inside(const glm::vec3 &p, float eps) const
{
	return 	   min.x - eps <= p.x && p.x <= max.x + eps
			&& min.y - eps <= p.y && p.y <= max.y + eps
			&& min.z - eps <= p.z && p.z <= max.z + eps;
}

/**
	This is only for compatibilty with other intersectable objects.

	\warning Returned normals are not real
*/
inline bool aabb::ray_intersect(const ray &r, ray_intersection &hit) const
{
	float t = ray_intersection_distance(r);
	if (t != HUGE_VALF)
	{
		hit.distance = t;
		hit.position = r.origin + t * r.direction;
		hit.direction = r.direction;
		hit.normal = -r.direction;
		return true;
	}
	
	return false;
}

/**
	Returns true if ray intersects AABB
*/
inline bool aabb::check_ray_intersect(const ray &r) const
{
	return ray_intersection_distance(r) != HUGE_VALF;
}

/**
	Returns closest distance to ray-box intersection. Returns HUGE_VALF on miss

	Based on: https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
*/
inline float aabb::ray_intersection_distance(const ray &r) const
{
	glm::vec3 a{(min - r.origin) / r.direction};
	glm::vec3 b{(max - r.origin) / r.direction};

	float tmin = std::max(std::max(std::min(a.x, b.x), std::min(a.y, b.y)), std::min(a.z, b.z));
	float tmax = std::min(std::min(std::max(a.x, b.x), std::max(a.y, b.y)), std::max(a.z, b.z));

	if (tmax < 0 || tmin > tmax) return HUGE_VALF;
	else return tmin;
}

/**
	Abstract base class for anything that can be bounded with AABB
*/
class aabb_provider
{
public:
	virtual aabb get_aabb() const = 0;
};

}