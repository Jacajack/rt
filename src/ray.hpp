#pragma once
#include <glm/glm.hpp>
#include "material.hpp"

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
	Carries information about ray-object intersection
*/
struct ray_bounce
{

};

}