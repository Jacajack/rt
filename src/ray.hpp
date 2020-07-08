#pragma once
#include <glm/glm.hpp>

namespace rt {

/**
	A ray....
*/
struct ray
{
	ray() = default;

	ray(const glm::vec3 &o, const glm::vec3 &d) :
		origin(o),
		direction(glm::normalize(d))
	{}

	glm::vec3 origin;
	glm::vec3 direction;
};


/**
	Distance to intersection returned when the ray misses
*/
inline constexpr float ray_miss = HUGE_VALF;

/**
	Contains minimal information about ray-object intersection. This infomation
	can later be used to generate ray_hit structure containing all infotmation
	necessary for shading.
*/
struct ray_intersection
{
	//! Distance to the intersection
	float distance;

	/**
		A helper pair of coordinates, later used for normal and UV calculations.
	*/
	float u, v;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
	}

	inline bool operator<(float x) const
	{
		return distance < x;
	}
};

/**
	Abstract base for providing ray_intersect functionality.
*/
class ray_intersectable
{
public:
	virtual inline bool ray_intersect(const ray &r, ray_intersection &hit) const = 0;
};

/**
	Determines reflective, transmissive and emissive properties of
	a surface. Also determines weights of outgoing (possibly branching)
	rays.

	Acquired from ray_hit structure by calling get_bounced_ray()
*/
struct ray_bounce
{
	//! Reflected ray
	ray reflected_ray;
	glm::vec3 brdf;
	float reflection_pdf;

	//! Transmitted ray
	ray transmitted_ray;
	glm::vec3 btdf;
	float transmission_ior;

	//! Emission value
	glm::vec3 emission;
};

/**
	Represents a ray branching point - stores parent
	ray's parameters and the new ray to be explored,
	so tracing can be later continued from the branching point

	\warning Deprecated - and will likely be removed
*/
struct ray_branch
{
	ray_branch() = default;

	ray_branch(const rt::ray &R, const glm::vec3 &w, float i, int d) :
		r(R),
		weight(w),
		ior(i),
		depth(d)
	{}

	//! The new ray
	rt::ray r;

	//! Parent ray's influence on pixel color
	glm::vec3 weight;

	//! IOR of the parent ray
	float ior;

	//! Depth
	int depth;
};

// Forward declarations of material and scene object
class abstract_material;
class scene_object;
class path_tracer;

/**
	Built from ray_intersection.
*/
struct ray_hit
{
	float distance;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 normal;

	const abstract_material *material;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
	}

	/**
		Based on two uniformly distributed random variables,
		returns scattering and emission information and new rays
		to be sampled.
	*/
	ray_bounce get_bounce(const rt::path_tracer &ctx, float ior) const;
};

}