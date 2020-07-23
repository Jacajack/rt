#pragma once

#include "../material.hpp"

namespace rt {

class general_bsdf : public abstract_material
{
public:
	// general_bsdf(const glm::vec3 &albedo, float roughness = 0.5f, float metallic = 0.f, float tranmission = 0.f, const glm::vec3 &emission);

	rt::ray_bounce get_bounce(const rt::path_tracer &ctx, const ray_hit &hit, float ior) const override;

	glm::vec3 base_color = glm::vec3{0.9};
	glm::vec3 emission = glm::vec3{0.f};
	float roughness = 0.5f;
	float metallic = 0.f;
	float transmission = 0.f;
	float ior = 1.5f;
};

}