#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"

namespace rt {

/**
	Materials are a way of providing scattering and emission information
	for ray_hit ray intersection points.
*/
class abstract_material
{
public:
	abstract_material() = default;

	abstract_material(const abstract_material &) = default;
	abstract_material(abstract_material &&) = default;

	abstract_material &operator=(const abstract_material &) = default;
	abstract_material &operator=(abstract_material &&) = default;

	virtual ~abstract_material() = default;

	/**
		Returns scattering and emission information and generates new rays based on two
		uniformly random numbers and IOR of the medium the ray was travelling in
	*/
	virtual rt::ray_bounce get_bounce(const rt::path_tracer &ctx, const ray_hit &hit, float ior) const = 0;
};

}