#pragma once
#include <glm/glm.hpp>

#include "bsdf.hpp"

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
};

/**
	Carries information about ray-object intersection and light transportation information
*/
struct ray_hit : public ray_intersection
{
	abstract_brdf *brdf;
};

/**
	Abstract base for providing ray_intersect functionality.
*/
class ray_intersectable
{
public:
	virtual inline bool ray_intersect(const ray &r, ray_intersection &hit) const = 0;
};

}