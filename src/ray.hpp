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
	Contains purely geometrical information about ray-object intersection
*/
struct ray_intersection
{
	float distance;
	glm::vec3 direction;
	glm::vec3 position;
	glm::vec3 normal;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
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
	Contains a ray bounced-off a surface and its contribution to the lighting

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

	//! Emission value
	glm::vec3 emission;
};

// Forward declarations of material and scene object
class abstract_material;
class scene_object;

/**
	ray_intersection + pointers to material, geometry and object
*/
struct ray_hit : public ray_intersection
{
	const abstract_material *material;
	const scene_object *object;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
	}

	/**
		Based on two uniformly distributed random variables,
		returns scattering and emission information and new rays
		to be sampled.
	*/
	ray_bounce get_bounce(float r1, float r2) const;
};

}