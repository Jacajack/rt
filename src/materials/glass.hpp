#pragma once

#include "../material.hpp"
#include "../path_tracer.hpp"

namespace rt {

class simple_glass_material : public abstract_material
{
public:
	simple_glass_material(const glm::vec3 color, float ior) :
		m_color(color),
		m_ior(ior)
	{}

	rt::ray_bounce get_bounce(const rt::path_tracer &ctx, const rt::ray_hit &hit, float ior) const override;

private:
	glm::vec3 m_color;
	float m_ior;
};

inline rt::ray_bounce simple_glass_material::get_bounce(const rt::path_tracer &ctx, const rt::ray_hit &hit, float ior) const
{
	float eta;
	float new_ior;
	glm::vec3 T, N;

	if (glm::dot(hit.direction, hit.normal) < 0.f)
	{
		// in
		eta = 1.f / m_ior;
		N = hit.normal;
		new_ior = m_ior;
	}
	else
	{
		// out
		eta = m_ior;
		N = -hit.normal;
		new_ior = 1.f;
	}

	T = glm::refract(hit.direction, N, eta);
	
	float F0 = ((1 - eta) * (1 - eta)) / ((1 + eta) * (1 + eta));
	float fresnel = F0 + (1.f - F0) * std::pow(1.f - glm::max(glm::dot(-hit.direction, N), 0.f), 5.f);

	// TIR case
	if (glm::length(T) == 0.f)
		fresnel = 1.f;

	// New ray and bounce
	rt::ray_bounce bounce;

	// Fresnel term determines amount of REFLECTED light
	if (ctx.get_rand() < fresnel)
	{
		// Reflect
		bounce.new_ray.origin = hit.position + N * 0.001f;
		bounce.new_ray.direction = glm::reflect(hit.direction, N);
		bounce.bsdf = glm::vec3{1.f};
		bounce.ior = 1.f;
	}
	else
	{
		// Transmit
		bounce.new_ray.origin = hit.position + hit.direction * 0.001f;
		bounce.new_ray.direction = T;
		bounce.bsdf = glm::vec3{m_color};
		bounce.ior = new_ior;
	}

	bounce.emission = glm::vec3{0.f};

	return bounce;
}

}