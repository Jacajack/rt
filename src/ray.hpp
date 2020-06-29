#pragma once
#include <glm/glm.hpp>

namespace rt {

/**
	A ray....
*/
struct ray
{
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
	ray_bounce(const ray &r, const glm::vec3 &fact) :
		new_ray(r),
		factor(fact)
	{}

	ray new_ray;

	/**
		This factor is computed from surface's BSDF and PDF and
		determines bounced off ray's contribution to the lighting
		of the surface it has bounced off. The cosine factor is also
		accounted here.
	*/
	glm::vec3 factor;
};

// Forward declarations of material and scene object
class material;
class scene_object;

/**
	ray_intersection + pointers to material, geometry and object
*/
struct ray_hit : public ray_intersection
{
	const ray_intersectable *geometry;
	const material *mat;
	const scene_object *object;

	inline bool operator<(const ray_intersection &rhs) const
	{
		return distance < rhs.distance;
	}

	/**
		Based on two uniformly distributed random variables,
		returns a ray bounced off the surface (in future, taking the
		BRDF PDF into account) and its contribution to the lighting.
	*/
	ray_bounce get_bounced_ray(float r1, float r2) const;
};

}