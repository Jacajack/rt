#pragma once

#include "../material.hpp"

namespace rt {

class simple_glass_material : public abstract_material
{
public:
	simple_glass_material(const glm::vec3 color, float ior) :
		m_color(color),
		m_ior(ior)
	{}

	rt::ray_bounce get_bounce(const rt::ray_hit &hit, float ior, float r1, float r2) const override;

private:
	glm::vec3 m_color;
	float m_ior;
};

inline rt::ray_bounce simple_glass_material::get_bounce(const rt::ray_hit &hit, float ior, float r1, float r2) const
{
	float eta;
	float new_ior;
	glm::vec3 T, N;

	if (glm::dot(hit.direction, hit.normal) < 0.f)
	{
		// in
		eta = 0.66f;
		N = hit.normal;
		new_ior = m_ior;
	}
	else
	{
		// out
		eta = 1.5f;
		N = -hit.normal;
		new_ior = 1.f;
	}

	T = glm::refract(hit.direction, N, eta);
	
	float F0 = ((1 - eta) * (1 - eta)) / ((1 + eta) * (1 + eta));
	float fresnel = F0 + (1.f - F0) * std::pow(1.f - glm::max(glm::dot(-hit.direction, N), 0.f), 5.f);

	// TIR case
	if (T.length() == 0.f)
		fresnel = 1.f;

	// Reflected ray
	rt::ray rr;
	rr.origin = hit.position + N * 0.001f;
	rr.direction = glm::reflect(hit.direction, N);

	// New ray
	rt::ray r;
	r.origin = hit.position + hit.direction * 0.001f;
	r.direction = T;

	rt::ray_bounce bounce;
	bounce.brdf = glm::vec3{fresnel};
	bounce.reflection_pdf = 1.f;
	bounce.reflected_ray = rr;

	bounce.transmission_ior = new_ior;
	bounce.btdf = m_color * (1.f - fresnel);
	bounce.transmitted_ray = r;

	bounce.emission = glm::vec3{0.f};

	return bounce;
}

}