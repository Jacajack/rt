#pragma once
#include "../material.hpp"

namespace rt {

/**
	Material for physically based rendering. Implements Cook-Torrance BRDF.
*/
class pbr_material : public abstract_material
{
public:
	pbr_material(const glm::vec3 &color, float roughness, float metallic = 0.f, const glm::vec3 &emission = glm::vec3{0.f}) :
		m_color(color),
		m_roughness(roughness),
		m_alpha(m_roughness * m_roughness),
		m_metallic(metallic),
		m_emission(emission)
	{}

	rt::ray_bounce get_bounce(const rt::ray_hit &hit, float ior, float r1, float r2) const override;


private:

	glm::vec3 m_color;
	float m_roughness;
	float m_alpha;
	float m_metallic;
	glm::vec3 m_emission;
};

}