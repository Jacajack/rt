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

}