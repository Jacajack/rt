#pragma once
#include "material.hpp"

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
		m_metallic(metallic),
		m_emission(emission)
	{}

	rt::ray_bounce get_bounce(const rt::ray_hit &hit, float r1, float r2) const override;


private:
	inline glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &wi) const;
	inline glm::vec3 fresnel(const glm::vec3 &H, const glm::vec3 &V, const glm::vec3 &F0) const;
	inline float schlick_ggx(float n_dot_v, float k) const;
	inline float geometry(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N, float k) const;
	inline float ndf(const glm::vec3 &H, const glm::vec3 &N, float k) const;

	glm::vec3 m_color;
	float m_roughness;
	float m_metallic;
	glm::vec3 m_emission;
};

}