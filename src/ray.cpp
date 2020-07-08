#include "ray.hpp"
#include "material.hpp"
#include "utility.hpp"

using rt::ray_hit;

rt::ray_bounce ray_hit::get_bounce(const rt::path_tracer &ctx, float ior) const
{
	return this->material->get_bounce(ctx, *this, ior);
}