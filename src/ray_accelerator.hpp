#pragma once

#include "ray.hpp"

namespace rt {

/**
	Abstract base for all ray intersection search accelerators
*/
class ray_accelerator
{
public:
	virtual bool cast_ray(const rt::ray &r, ray_hit &hit) const = 0;
};

}