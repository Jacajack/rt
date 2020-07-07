#pragma once 

#include "../material.hpp"

namespace rt {

/**
	A simple sky material that can be used by the scene
*/
class simple_sky_material : public abstract_material
{
public:

	rt::ray_bounce get_bounce(const ray_hit &hit, float ior, float r1, float r2) const override
	{
		rt::ray_bounce bounce;
		bounce.brdf = glm::vec3{0.f};
		bounce.reflection_pdf = 1.f;
		bounce.btdf = glm::vec3{0.f};
		// bounce.emission = glm::vec3{0.5, 0.5, 1.0} + 10.f * glm::vec3{std::pow(glm::dot(hit.direction, {0, 1, 0}), 4.f)}; 
		bounce.emission = 1.f * glm::mix(glm::vec3{0.3, 0.3, 0.4}, glm::vec3{1.1, 1.0, 2.0}, std::abs(hit.direction.y));
		return bounce;
	}
};

}