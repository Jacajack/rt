#include "ray.hpp"
#include "material.hpp"
#include "utility.hpp"

using rt::ray_hit;

rt::ray_bounce ray_hit::get_bounce(float ior, float r1, float r2) const
{
	return this->material->get_bounce(*this, ior, r1, r2);
}