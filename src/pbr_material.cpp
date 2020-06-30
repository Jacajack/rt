#include "pbr_material.hpp"
#include "utility.hpp"

using rt::pbr_material;

/**
	Cook-Torrance specular BRDF + Lambertian diffuse
*/
glm::vec3 pbr_material::brdf(const ray_hit &hit, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const
{
	// Test values
	float alpha = 0.01f;
	float metallic = 0.0f;

	// The halfway vector
	glm::vec3 H = glm::normalize(L + V);

	// Estimated surface's response at normal incidence
	glm::vec3 F0 = glm::mix(glm::vec3{0.04f}, m_color, metallic);

	// Specular and diffuse components
	glm::vec3 k_specular = fresnel(H, V, F0);
	glm::vec3 k_diffuse = (1.f - k_specular) * (1.f - metallic);

	// k for Schlick-GGX
	float k = std::pow(alpha + 1.f, 2.f) / 8.f;

	// Lambert and Cook-Torrance terms
	float n_dot_l = glm::max(glm::dot(N, L), 0.f);
	float n_dot_v = glm::max(glm::dot(N, V), 0.f);
	glm::vec3 lambert{m_color * n_dot_l / rt::pi<>};
	glm::vec3 cook_torrance{ndf(H, N, alpha) * geometry(L, V, N, k) / glm::max(4.f * n_dot_l * n_dot_v, 0.001f)};
	return k_specular * cook_torrance + k_diffuse * lambert;
}

/**
	Fresnel factor (Schlick's approximation)
*/
glm::vec3 pbr_material::fresnel(const glm::vec3 &H, const glm::vec3 &V, const glm::vec3 &F0) const
{
	return F0 + (1.f - F0) * std::pow(1.f - glm::max(glm::dot(H, V), 0.f), 5.f);
}

/**
	Sclick GGX geometry shadowing function
*/
float pbr_material::schlick_ggx(float n_dot_v, float k) const
{
	return n_dot_v / (n_dot_v * (1.f - k) + k);
}

/**
	Geometry function - Schlick GGX + Smith's method
*/
float pbr_material::geometry(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N, float k) const
{
	float n_dot_l = glm::max(glm::dot(N, L), 0.f);
	float n_dot_v = glm::max(glm::dot(N, V), 0.f);
	return schlick_ggx(n_dot_l, k) * schlick_ggx(n_dot_v, k);
}

/**
	Trowbridge-Reitz (GGX) normal distribution function
*/
float pbr_material::ndf(const glm::vec3 &H, const glm::vec3 &N, float alpha) const
{
	float alpha_sq = alpha * alpha;
	float n_dot_h = glm::max(glm::dot(N, H), 0.f);
	float denominator = n_dot_h * n_dot_h * (alpha_sq - 1.f) + 1.f;
	return alpha_sq / (rt::pi<> * denominator * denominator);
}